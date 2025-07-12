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

static int shell_stdin_fd = -1;
static int shell_stdout_fd = -1;
static std::thread reader_thread;
static std::mutex output_mutex;
static std::vector<std::string> shell_output;
static std::atomic<bool> running{false};

// Function to read asynchronously from shell stdout
static void read_from_shell() {
    char buffer[256];
    while (running) {
        ssize_t n = read(shell_stdout_fd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = 0;
            std::lock_guard<std::mutex> lock(output_mutex);
            shell_output.push_back(std::string(buffer));
        } else {
            usleep(10000); // sleep 10ms if no data
        }
    }
}

// Initialize and start the shell subprocess.
// Returns true on success, false otherwise.
bool start_shell() {
    int stdin_pipe[2];
    int stdout_pipe[2];
    if (pipe(stdin_pipe) < 0 || pipe(stdout_pipe) < 0)
        return false;

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stdout_pipe[1], STDERR_FILENO);
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        execl("/bin/bash", "/bin/bash", nullptr);
        _exit(1); // exec failed
    } else if (pid > 0) {
        // Parent process
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);
        shell_stdin_fd = stdin_pipe[1];
        shell_stdout_fd = stdout_pipe[0];

        // Set non-blocking read on stdout
        fcntl(shell_stdout_fd, F_SETFL, O_NONBLOCK);

        running = true;
        reader_thread = std::thread(read_from_shell);

        return true;
    }
    return false;
}

// Shutdown the shell subprocess and clean up resources.
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

// Send input text (a command) to the shell.
void write_to_shell(const std::string& input) {
        shell_output.push_back(">" + input); // Add to output buffer immediately
        write(shell_stdin_fd, input.c_str(), input.size());
        write(shell_stdin_fd, "\n", 1);
}

// Get a snapshot of the shell output.
// Thread-safe: copies internal output buffer under mutex.
std::vector<std::string> get_shell_output() {
    std::lock_guard<std::mutex> lock(output_mutex);
    return shell_output;
}

// Clear the shell output buffer.
void clear_shell_output() {
    std::lock_guard<std::mutex> lock(output_mutex);
    shell_output.clear();
}
