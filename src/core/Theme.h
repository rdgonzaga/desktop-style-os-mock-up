#pragma once

#include "imgui.h"

namespace theme {

void apply();
void loadFonts();
ImFont* mono();

// ImGui draws title text in the body text color, so these swap in white for the title bar
bool beginWindow(const char* title, bool* open, ImGuiWindowFlags flags = 0);
bool beginModal(const char* title, ImGuiWindowFlags flags = 0);

}
