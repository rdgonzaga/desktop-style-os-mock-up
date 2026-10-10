#include "core/Theme.h"

#include <filesystem>

namespace theme {

namespace {

ImVec4 rgb(int r, int g, int b, float a = 1.0f) {
    return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a);
}

const ImVec4 TITLE_TEXT = rgb(255, 255, 255);

ImFont* monoFont = nullptr;

ImFont* addFont(const char* path, float size) {
    ImGuiIO& io = ImGui::GetIO();
    if (std::filesystem::exists(path)) {
        return io.Fonts->AddFontFromFileTTF(path, size);
    }
    ImFontConfig font;
    font.SizePixels = size;
    return io.Fonts->AddFontDefaultVector(&font);
}

}

void apply() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 10.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 6.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;
    style.WindowPadding = ImVec2(12.0f, 12.0f);
    style.FramePadding = ImVec2(10.0f, 6.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 6.0f);
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 12.0f;
    style.WindowTitleAlign = ImVec2(0.02f, 0.5f);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = rgb(240, 240, 240);
    colors[ImGuiCol_TextDisabled] = rgb(140, 140, 140);
    colors[ImGuiCol_WindowBg] = rgb(32, 32, 32, 0.98f);
    colors[ImGuiCol_ChildBg] = rgb(26, 26, 26, 0.70f);
    colors[ImGuiCol_PopupBg] = rgb(36, 36, 36, 0.98f);
    colors[ImGuiCol_Border] = rgb(62, 62, 62, 0.85f);
    colors[ImGuiCol_BorderShadow] = rgb(0, 0, 0, 0.0f);
    colors[ImGuiCol_FrameBg] = rgb(44, 44, 44);
    colors[ImGuiCol_FrameBgHovered] = rgb(56, 56, 56);
    colors[ImGuiCol_FrameBgActive] = rgb(64, 64, 64);
    colors[ImGuiCol_TitleBg] = rgb(26, 26, 26);
    colors[ImGuiCol_TitleBgActive] = rgb(34, 34, 34);
    colors[ImGuiCol_TitleBgCollapsed] = rgb(24, 24, 24);
    colors[ImGuiCol_MenuBarBg] = rgb(32, 32, 32);
    colors[ImGuiCol_ScrollbarBg] = rgb(28, 28, 28, 0.50f);
    colors[ImGuiCol_ScrollbarGrab] = rgb(80, 80, 80);
    colors[ImGuiCol_ScrollbarGrabHovered] = rgb(110, 110, 110);
    colors[ImGuiCol_ScrollbarGrabActive] = rgb(140, 140, 140);
    colors[ImGuiCol_CheckMark] = rgb(0, 120, 215);
    colors[ImGuiCol_SliderGrab] = rgb(0, 120, 215);
    colors[ImGuiCol_SliderGrabActive] = rgb(0, 103, 192);
    colors[ImGuiCol_Button] = rgb(45, 45, 45);
    colors[ImGuiCol_ButtonHovered] = rgb(58, 58, 58);
    colors[ImGuiCol_ButtonActive] = rgb(38, 38, 38);
    colors[ImGuiCol_Header] = rgb(0, 120, 215, 0.35f);
    colors[ImGuiCol_HeaderHovered] = rgb(0, 120, 215, 0.55f);
    colors[ImGuiCol_HeaderActive] = rgb(0, 120, 215, 0.75f);
    colors[ImGuiCol_Separator] = rgb(55, 55, 55);
    colors[ImGuiCol_SeparatorHovered] = rgb(0, 120, 215, 0.78f);
    colors[ImGuiCol_SeparatorActive] = rgb(0, 120, 215);
    colors[ImGuiCol_ResizeGrip] = rgb(80, 80, 80, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = rgb(0, 120, 215, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = rgb(0, 120, 215);
    colors[ImGuiCol_Tab] = rgb(36, 36, 36);
    colors[ImGuiCol_TabHovered] = rgb(50, 50, 50);
    colors[ImGuiCol_TabActive] = rgb(0, 120, 215, 0.85f);
    colors[ImGuiCol_TabUnfocused] = rgb(30, 30, 30);
    colors[ImGuiCol_TabUnfocusedActive] = rgb(42, 42, 42);
    colors[ImGuiCol_TableHeaderBg] = rgb(40, 40, 40);
    colors[ImGuiCol_TableBorderStrong] = rgb(55, 55, 55);
    colors[ImGuiCol_TableBorderLight] = rgb(45, 45, 45);
    colors[ImGuiCol_TableRowBg] = rgb(0, 0, 0, 0.0f);
    colors[ImGuiCol_TableRowBgAlt] = rgb(255, 255, 255, 0.03f);
    colors[ImGuiCol_TextSelectedBg] = rgb(0, 120, 215, 0.45f);
    colors[ImGuiCol_DragDropTarget] = rgb(0, 120, 215);
    colors[ImGuiCol_NavHighlight] = rgb(0, 120, 215);
    colors[ImGuiCol_ModalWindowDimBg] = rgb(0, 0, 0, 0.60f);
}

void loadFonts() {
    if (std::filesystem::exists("assets/fonts/Inter.ttf")) {
        addFont("assets/fonts/Inter.ttf", 16.0f);
    } else {
#if defined(__APPLE__)
        addFont("/System/Library/Fonts/SFNS.ttf", 16.0f);
#else
        addFont("C:/Windows/Fonts/segoeui.ttf", 16.0f);
#endif
    }

    if (std::filesystem::exists("assets/fonts/JetBrainsMono.ttf")) {
        monoFont = addFont("assets/fonts/JetBrainsMono.ttf", 15.0f);
    } else {
#if defined(__APPLE__)
        monoFont = addFont("/System/Library/Fonts/SFNSMono.ttf", 15.0f);
#else
        monoFont = addFont("C:/Windows/Fonts/consola.ttf", 16.0f);
#endif
    }
}

ImFont* mono() {
    return monoFont;
}

bool beginWindow(const char* title, bool* open, ImGuiWindowFlags flags) {
    ImGui::PushStyleColor(ImGuiCol_Text, TITLE_TEXT);
    bool visible = ImGui::Begin(title, open, flags);
    ImGui::PopStyleColor();
    return visible;
}

bool beginModal(const char* title, ImGuiWindowFlags flags) {
    ImGui::PushStyleColor(ImGuiCol_Text, TITLE_TEXT);
    bool open = ImGui::BeginPopupModal(title, nullptr, flags);
    ImGui::PopStyleColor();
    return open;
}

}
