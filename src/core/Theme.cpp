#include "core/Theme.h"

namespace theme {

namespace {

ImVec4 rgb(int r, int g, int b, float a = 1.0f) {
    return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a);
}

const ImVec4 TITLE_TEXT = rgb(255, 255, 255);

}

void apply() {
    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 3.0f;
    style.ScrollbarRounding = 6.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.WindowPadding = ImVec2(10.0f, 10.0f);
    style.FramePadding = ImVec2(8.0f, 4.0f);
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = rgb(0, 0, 0);
    colors[ImGuiCol_WindowBg] = rgb(236, 233, 216);
    colors[ImGuiCol_PopupBg] = rgb(255, 255, 255);
    colors[ImGuiCol_Border] = rgb(0, 60, 116, 0.6f);
    colors[ImGuiCol_TitleBg] = rgb(122, 150, 223);
    colors[ImGuiCol_TitleBgActive] = rgb(0, 84, 227);
    colors[ImGuiCol_TitleBgCollapsed] = rgb(122, 150, 223);
    colors[ImGuiCol_FrameBg] = rgb(255, 255, 255);
    colors[ImGuiCol_FrameBgHovered] = rgb(255, 255, 255);
    colors[ImGuiCol_FrameBgActive] = rgb(255, 255, 255);
    colors[ImGuiCol_Button] = rgb(245, 244, 239);
    colors[ImGuiCol_ButtonHovered] = rgb(253, 236, 194);
    colors[ImGuiCol_ButtonActive] = rgb(226, 223, 205);
    colors[ImGuiCol_Header] = rgb(193, 210, 238);
    colors[ImGuiCol_HeaderHovered] = rgb(213, 226, 247);
    colors[ImGuiCol_HeaderActive] = rgb(170, 195, 235);
    colors[ImGuiCol_ModalWindowDimBg] = rgb(0, 0, 0, 0.35f);
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
