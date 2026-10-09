#pragma once

#include "imgui.h"

// Windows XP "Luna" look, to match the wallpaper.
namespace theme {

void apply();

// ImGui draws title text in the same color as body text, so XP's white text on a
// blue title bar needs the color swapped just while the title is drawn.
bool beginModal(const char* title, ImGuiWindowFlags flags = 0);

}
