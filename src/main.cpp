#include <cstdio>
#include <filesystem>

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Config.h"
#include "core/Apps.h"
#include "core/Power.h"
#include "core/Theme.h"
#include "shell/Boot.h"
#include "shell/Desktop.h"
#include "shell/Taskbar.h"

int main(int, char** argv) {
    // so assets/ and config.txt are found no matter where the exe is started from
    std::error_code ignored;
    std::filesystem::current_path(std::filesystem::absolute(argv[0]).parent_path(), ignored);
    loadConfig("config.txt");

    if (!glfwInit()) {
        std::fprintf(stderr, "failed to start GLFW\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWmonitor* monitor = nullptr;
    int windowWidth = config.windowWidth;
    int windowHeight = config.windowHeight;
    if (config.fullscreen) {
        monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        windowWidth = mode->width;
        windowHeight = mode->height;
    }
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "CSOPESY Desktop OS Emulator", monitor, nullptr);
    if (!window) {
        std::fprintf(stderr, "failed to create the window\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    // only the PWR button may close the app, so X and Alt+F4 are ignored
    glfwSetWindowCloseCallback(window, [](GLFWwindow* w) { glfwSetWindowShouldClose(w, GLFW_FALSE); });

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    theme::apply();
    theme::loadFonts();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    desktop::init();
    power::state = config.bootScreens ? power::State::Booting : power::State::Running;

    while (power::state != power::State::Off) {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (power::state == power::State::Booting) {
            boot::draw();
        } else {
            // back to front
            desktop::draw();
            apps::drawWindows();
            taskbar::draw();
        }

        ImGui::Render();
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    desktop::shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
