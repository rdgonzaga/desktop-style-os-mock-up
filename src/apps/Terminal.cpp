#include "apps/Terminal.h"

#include "imgui.h"

namespace terminal {

void draw() {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(12, 12, 12, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(204, 204, 204, 255));
    ImGui::BeginChild("screen");
    ImGui::TextUnformatted("CSOPESY OS [Version 1.0]");
    ImGui::TextUnformatted("(c) 2026 CSOPESY Group. All rights reserved.");
    ImGui::NewLine();
    ImGui::TextUnformatted("C:\\Users\\csopesy>");
    ImGui::EndChild();
    ImGui::PopStyleColor(2);
}

}
