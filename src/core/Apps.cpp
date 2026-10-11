#include "core/Apps.h"

#include <algorithm>
#include <cctype>

#include "apps/FileExplorer.h"
#include "apps/Notepad.h"
#include "apps/Settings.h"
#include "apps/TaskManager.h"
#include "apps/Terminal.h"
#include "core/Theme.h"
#include "shell/Taskbar.h"

namespace apps {

namespace {

std::vector<App> table = {
    {"Terminal", Icon::Terminal, &config.terminalSize, terminal::draw},
    {"File Explorer", Icon::Folder, &config.explorerSize, explorer::draw},
    {"Task Manager", Icon::TaskManager, &config.taskManagerSize, taskmanager::draw, ImGuiWindowFlags_MenuBar},
    {"Notepad", Icon::Notepad, &config.notepadSize, notepad::draw, ImGuiWindowFlags_MenuBar},
    {"Settings", Icon::Settings, &config.settingsSize, settings::draw},
};

const App* active = nullptr;

void minimize(App& app) {
    app.minimized = true;
    if (active == &app) {
        active = nullptr;
    }
}

std::string simplified(const std::string& name) {
    std::string out;
    for (char c : name) {
        if (c != ' ') {
            out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
    }
    return out;
}

void clampToWorkArea(ImVec2 workMin, ImVec2 workMax) {
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();
    ImVec2 clamped(std::max(workMin.x, std::min(pos.x, workMax.x - size.x)),
                   std::max(workMin.y, std::min(pos.y, workMax.y - size.y)));
    if (clamped.x != pos.x || clamped.y != pos.y) {
        ImGui::SetWindowPos(clamped);
    }
}

}

std::vector<App>& all() {
    return table;
}

App* find(const std::string& name) {
    std::string wanted = simplified(name);
    if (wanted.empty()) {
        return nullptr;
    }
    for (App& app : table) {
        if (simplified(app.name) == wanted) {
            return &app;
        }
    }
    for (App& app : table) {
        if (simplified(app.name).find(wanted) != std::string::npos) {
            return &app;
        }
    }
    return nullptr;
}

bool isActive(const App& app) {
    return active == &app;
}

void open(App& app) {
    app.open = true;
    app.minimized = false;
    app.focusRequested = true;
}

void close(App& app) {
    app.open = false;
    if (active == &app) {
        active = nullptr;
    }
}

void toggle(App& app) {
    if (app.open && !app.minimized && isActive(app)) {
        minimize(app);
    } else {
        open(app);
    }
}

void drawWindows() {
    ImVec2 workMin = taskbar::workMin();
    ImVec2 workMax = taskbar::workMax();

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
        active = nullptr;
    }

    float cascade = 0.0f;
    for (App& app : table) {
        ImVec2 firstPos(80.0f + cascade, workMin.y + 70.0f + cascade);
        cascade += 40.0f;
        if (!app.open || app.minimized) {
            continue;
        }

        ImGui::SetNextWindowPos(firstPos, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(app.size->width), static_cast<float>(app.size->height)),
                                 ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(360, 220), workMax - workMin);
        if (app.focusRequested) {
            ImGui::SetNextWindowFocus();
            app.focusRequested = false;
        }

        if (theme::beginWindow(app.name, &app.open, ImGuiWindowFlags_NoCollapse | app.windowFlags)) {
            clampToWorkArea(workMin, workMax);
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
                active = &app;
            }
            app.drawContent();
        }
        ImGui::End();

        if (!app.open && active == &app) {
            active = nullptr;
        }
    }
}

}
