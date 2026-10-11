#include "apps/Notepad.h"

#include <algorithm>
#include <cstring>

#include "imgui.h"

#include "core/Apps.h"
#include "core/Theme.h"

namespace notepad {

namespace {

char text[16384] = "";
bool wrap = true;

int lineCount() {
    return static_cast<int>(std::count(text, text + std::strlen(text), '\n')) + 1;
}

void menuBar() {
    if (!ImGui::BeginMenuBar()) {
        return;
    }
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New")) {
            text[0] = '\0';
        }
        if (ImGui::MenuItem("Exit")) {
            if (App* self = apps::find("Notepad")) {
                apps::close(*self);
            }
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Format")) {
        ImGui::MenuItem("Word Wrap", nullptr, &wrap);
        ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
}

}

void draw() {
    menuBar();
    float footer = ImGui::GetFrameHeightWithSpacing();
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
    if (wrap) {
        flags |= ImGuiInputTextFlags_WordWrap;
    }
    ImGui::PushFont(theme::mono(), 0.0f);
    ImGui::InputTextMultiline("##text", text, sizeof(text), ImVec2(-FLT_MIN, -footer), flags);
    ImGui::PopFont();
    ImGui::TextDisabled("%d lines, %d characters", lineCount(), static_cast<int>(std::strlen(text)));
}

}
