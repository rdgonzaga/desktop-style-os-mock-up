#include "apps/TaskManager.h"

#include "imgui.h"

namespace taskmanager {

void draw() {
    ImGui::TextUnformatted("Processes");
    ImGui::Separator();
    ImGui::TextDisabled("No processes to show yet.");
}

}
