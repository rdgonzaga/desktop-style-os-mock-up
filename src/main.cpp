#include <cstdio>
#include <filesystem>

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Config.h"
#include "core/Power.h"
#include "core/Theme.h"
#include "shell/Desktop.h"
#include "shell/Taskbar.h"

namespace {

void loadFont() {
    ImGuiIO& io = ImGui::GetIO();
    const char* segoe = "C:/Windows/Fonts/segoeui.ttf";
    if (std::filesystem::exists(segoe)) {
        io.Fonts->AddFontFromFileTTF(segoe, 17.0f);
    } else {
        ImFontConfig font;
        font.SizePixels = 17.0f;
        io.Fonts->AddFontDefaultVector(&font);
    }
}

}

int main(int, char** argv) {
    // run from the exe's own folder so assets/ is found however the app was launched
    std::error_code ignored;
    std::filesystem::current_path(std::filesystem::absolute(argv[0]).parent_path(), ignored);

    if (!glfwInit()) {
        std::fprintf(stderr, "failed to start GLFW\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(config.windowWidth, config.windowHeight,
                                          "CSOPESY Desktop OS Emulator", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "failed to create the window\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    // the X button and Alt+F4 do nothing; the spec wants PWR to be the only way out
    glfwSetWindowCloseCallback(window, [](GLFWwindow* w) { glfwSetWindowShouldClose(w, GLFW_FALSE); });

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;  // every boot starts with a fresh window layout
    theme::apply();
    loadFont();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    desktop::init();

    while (power::on) {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
            ImGui_ImplGlfw_Sleep(10);  // nothing to draw while minimized
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // layers are drawn back to front
        desktop::draw();
        taskbar::draw();

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
