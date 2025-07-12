#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "shell/shell.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <cstring> // for strlen

void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // macOS compatibility

    GLFWwindow* window = glfwCreateWindow(1280, 720, "TerminalAI", nullptr, nullptr);
    if (!window) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Input buffer
    int buf_size = 1024;
    char* buf = new char[buf_size];
    buf[0] = '\0';

    // Flags for input focus and scrolling
    bool should_refocus = true;
    bool scroll_to_bottom = false;

    // Start the shell process
    if (!start_shell()) {
        fprintf(stderr, "Failed to start shell process!\n");
        return 1;
    }

    // Main Loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)display_w, (float)display_h));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.10f, 1.00f));

        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("Main", nullptr, window_flags);

        ImGui::PushItemWidth(-FLT_MIN);

        // Display shell output from shell.cpp thread-safe buffer
        std::vector<std::string> messages = get_shell_output();

        ImGui::BeginChild("MessageRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

        for (const auto& msg : messages) {
            ImGui::TextWrapped("%s", msg.c_str());
        }

        if (scroll_to_bottom)
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();

        // Input box: when Enter pressed, send input to shell
        if (should_refocus)
            ImGui::SetKeyboardFocusHere();

        if (ImGui::InputText("##Input", buf, buf_size, ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (strlen(buf) > 0) {
                write_to_shell(std::string(buf));
                buf[0] = '\0';
                scroll_to_bottom = true;
            }
        } else {
            scroll_to_bottom = false;
        }

        ImGui::PopItemWidth();
        ImGui::End();

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();

        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.10f, 0.10f, 0.10f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup shell
    stop_shell();

    // Cleanup ImGui & GLFW
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    delete[] buf;

    return 0;
}
