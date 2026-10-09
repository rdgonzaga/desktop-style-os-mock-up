#pragma once

#include <vector>

#include "imgui.h"

#include "core/Icons.h"

struct App {
    const char* name;
    Icon icon;
    ImVec2 defaultSize;
    void (*drawContent)();

    bool open = false;
    bool minimized = false;
    bool focusRequested = false;
};

namespace apps {

std::vector<App>& all();
bool isActive(const App& app);

void open(App& app);
void toggle(App& app);

void drawWindows();

}
