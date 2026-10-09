#include "apps/FileExplorer.h"

#include "imgui.h"

#include "core/Icons.h"

namespace explorer {

void draw() {
    const char* places[] = {"Desktop", "Documents", "Downloads", "Pictures", "Local Disk (C:)"};

    ImGui::TextUnformatted("This PC");
    ImGui::Separator();
    for (const char* place : places) {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float line = ImGui::GetTextLineHeight();
        drawIcon(ImGui::GetWindowDrawList(), Icon::Folder, ImVec2(pos.x + 10, pos.y + line / 2), 20.0f);
        ImGui::SetCursorScreenPos(ImVec2(pos.x + 28, pos.y));
        ImGui::TextUnformatted(place);
    }
}

}
