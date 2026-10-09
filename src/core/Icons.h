#pragma once

#include "imgui.h"

enum class Icon { Terminal, Folder, TaskManager, Window, File, Drive, Computer };

void drawIcon(ImDrawList* draw, Icon icon, ImVec2 center, float size);
