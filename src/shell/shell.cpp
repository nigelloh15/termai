// shell.cpp
#include "shell.h"

#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <atomic>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <iostream>

#include <Python.h>

static int shell_stdin_fd = -1;
static int shell_stdout_fd = -1;
static std::thread reader_thread;
static std::mutex output_mutex;
static std::vector<std::string> shell_output;
static std::atomic<bool> running{false};

static PyObject* pModule = nullptr;
static PyObject* pGetResponseFunc = nullptr;

static std::mutex input_mutex;
static std::string last_input;

bool init_python() {
    Py_Initialize();

    std::cout << "Python version: " << Py_GetVersion() << std::endl;

    char cwd_buff[PATH_MAX];
    if (getcwd(cwd_buff, sizeof(cwd_buff)) == nullptr) {
        perror("getcwd error");
        return false;
    }

    std::string cwd(cwd_buff);
    std::string venv_site_packages = cwd + "/venv/lib/python3.12/site-packages";
    cwd.append("/python");

    PyRun_SimpleString("import site");

    std::string code =
        "import sys\n"
        "sys.path.insert(0, r'" + cwd + "')\n"
        "sys.path.insert(0, r'" + venv_site_packages + "')\n";
    PyRun_SimpleString(code.c_str());

    PyObject* pName = PyUnicode_DecodeFSDefault("integration");
    pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (!pModule) {
        PyErr_Print();
        std::cerr << "Failed to load Python module 'integration'\n";
        return false;
    }

    pGetResponseFunc = PyObject_GetAttrString(pModule, "getResponse");
    if (!pGetResponseFunc || !PyCallable_Check(pGetResponseFunc)) {
        PyErr_Print();
        std::cerr << "Cannot find callable 'getResponse' in module\n";
        return false;
    }

    return true;
}

void finalize_python() {
    Py_XDECREF(pGetResponseFunc);
    Py_XDECREF(pModule);
    if (Py_IsInitialized()) {
        Py_Finalize();
    }
}

// New C++ wrapper for Python getResponse
std::string getResponse(const std::string& input) {
    if (!pGetResponseFunc) return "[Python not initialized]";

    PyGILState_STATE gstate = PyGILState_Ensure();

    PyObject* pArgs = PyTuple_New(1);
    PyObject* pValue = PyUnicode_FromString(input.c_str());
    PyTuple_SetItem(pArgs, 0, pValue); // steals ref

    PyObject* pResult = PyObject_CallObject(pGetResponseFunc, pArgs);
    Py_DECREF(pArgs);

    std::string result = "[Python error]";
    if (pResult) {
        const char* resp_cstr = PyUnicode_AsUTF8(pResult);
        if (resp_cstr) result = std::string(resp_cstr);
        Py_DECREF(pResult);
    } else {
        PyErr_Print();
    }

    PyGILState_Release(gstate);
    return result;
}

static void read_from_shell() {
    char buffer[256];
    while (running) {
        ssize_t n = read(shell_stdout_fd, buffer, sizeof(buffer) - 1);

        if (n > 0) {
            buffer[n] = 0;
            std::lock_guard<std::mutex> lock(output_mutex);
            shell_output.push_back(std::string(buffer));
        } else if (n == 0) {
            running = false;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(10000);
            } else {
                std::string input_copy;
                {
                    std::lock_guard<std::mutex> lock(input_mutex);
                    input_copy = last_input;
                }

                std::string response = getResponse(input_copy);
                {
                    std::lock_guard<std::mutex> lock(output_mutex);
                    shell_output.push_back("[Python getResponse] " + response);
                }

                running = false;
            }
        }
    }
}

bool start_shell() {
    int stdin_pipe[2];
    int stdout_pipe[2];
    if (pipe(stdin_pipe) < 0 || pipe(stdout_pipe) < 0)
        return false;

    pid_t pid = fork();
    if (pid == 0) {

        // Change working directory to home
        const char* home_dir = getenv("HOME");
        if (home_dir) {
            chdir(home_dir);
        }

        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stdout_pipe[1], STDERR_FILENO);
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        execl("/bin/bash", "/bin/bash", nullptr);
        _exit(1);
    } else if (pid > 0) {
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);
        shell_stdin_fd = stdin_pipe[1];
        shell_stdout_fd = stdout_pipe[0];

        fcntl(shell_stdout_fd, F_SETFL, O_NONBLOCK);

        running = true;
        reader_thread = std::thread(read_from_shell);

        return true;
    }
    return false;
}

void stop_shell() {
    running = false;
    if (reader_thread.joinable()) {
        reader_thread.join();
    }

    if (shell_stdin_fd != -1) {
        close(shell_stdin_fd);
        shell_stdin_fd = -1;
    }
    if (shell_stdout_fd != -1) {
        close(shell_stdout_fd);
        shell_stdout_fd = -1;
    }

    std::lock_guard<std::mutex> lock(output_mutex);
    shell_output.clear();
}

void write_to_shell(const std::string& input) {
    {
        std::lock_guard<std::mutex> lock(input_mutex);
        last_input = input;
    }

    {
        std::lock_guard<std::mutex> lock(output_mutex);
        shell_output.push_back("> " + input);
    }

    if (shell_stdin_fd != -1) {
        write(shell_stdin_fd, input.c_str(), input.size());
        write(shell_stdin_fd, "\n", 1);
    }
}

std::vector<std::string>* get_shell_output() {
    return &shell_output;
}

void clear_shell_output() {
    std::lock_guard<std::mutex> lock(output_mutex);
    shell_output.clear();
}
