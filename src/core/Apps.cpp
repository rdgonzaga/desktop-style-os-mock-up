#include "core/Apps.h"

#include <algorithm>
#include <cctype>

#include "apps/FileExplorer.h"
#include "apps/TaskManager.h"
#include "apps/Terminal.h"
#include "core/Theme.h"
#include "shell/Taskbar.h"

namespace apps {

namespace {

std::vector<App> table = {
    {"Terminal", Icon::Terminal, ImVec2(640, 400), terminal::draw},
    {"File Explorer", Icon::Folder, ImVec2(720, 460), explorer::draw},
    {"Task Manager", Icon::TaskManager, ImVec2(780, 500), taskmanager::draw, ImGuiWindowFlags_MenuBar},
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

void clampToWorkArea(ImVec2 workMax) {
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();
    ImVec2 clamped(std::max(0.0f, std::min(pos.x, workMax.x - size.x)),
                   std::max(0.0f, std::min(pos.y, workMax.y - size.y)));
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
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    ImVec2 workMax(screen.x, screen.y - taskbar::HEIGHT);

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
        active = nullptr;
    }

    float cascade = 0.0f;
    for (App& app : table) {
        ImVec2 firstPos(80.0f + cascade, 70.0f + cascade);
        cascade += 40.0f;
        if (!app.open || app.minimized) {
            continue;
        }

        ImGui::SetNextWindowPos(firstPos, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(app.defaultSize, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(360, 220), workMax);
        if (app.focusRequested) {
            ImGui::SetNextWindowFocus();
            app.focusRequested = false;
        }

        if (theme::beginWindow(app.name, &app.open, ImGuiWindowFlags_NoCollapse | app.windowFlags)) {
            clampToWorkArea(workMax);
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
