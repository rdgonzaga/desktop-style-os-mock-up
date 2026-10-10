#include "shell/Desktop.h"

#include <string>

#include "imgui.h"

#include "Config.h"
#include "core/Apps.h"
#include "core/Clock.h"
#include "core/Icons.h"
#include "core/Texture.h"
#include "shell/Taskbar.h"

namespace desktop {

namespace {

const float MARGIN = 12.0f;
const ImVec2 BADGE_PADDING(10.0f, 5.0f);

Texture wallpaper;

// cropped instead of stretched when the window isn't 16:9
void drawWallpaper(ImDrawList* draw, ImVec2 screen) {
    if (!wallpaper.id) {
        ImU32 sky = IM_COL32(30, 80, 170, 255);
        ImU32 grass = IM_COL32(60, 130, 50, 255);
        draw->AddRectFilledMultiColor(ImVec2(0, 0), screen, sky, sky, grass, grass);
        return;
    }

    float imageAspect = static_cast<float>(wallpaper.width) / wallpaper.height;
    float screenAspect = screen.x / screen.y;
    ImVec2 uvMin(0, 0);
    ImVec2 uvMax(1, 1);
    if (screenAspect > imageAspect) {
        float visible = imageAspect / screenAspect;
        uvMin.y = (1 - visible) / 2;
        uvMax.y = 1 - uvMin.y;
    } else {
        float visible = screenAspect / imageAspect;
        uvMin.x = (1 - visible) / 2;
        uvMax.x = 1 - uvMin.x;
    }
    draw->AddImage(static_cast<ImTextureID>(wallpaper.id), ImVec2(0, 0), screen, uvMin, uvMax);
}

ImVec2 badgeSize(const std::string& text) {
    return ImGui::CalcTextSize(text.c_str()) + BADGE_PADDING * 2;
}

void drawBadge(ImDrawList* draw, ImVec2 pos, const std::string& text) {
    ImVec2 size = badgeSize(text);
    draw->AddRectFilled(pos, pos + size, IM_COL32(24, 24, 24, 195), 8.0f);
    draw->AddRect(pos, pos + size, IM_COL32(255, 255, 255, 30), 8.0f, 0, 1.0f);
    draw->AddText(pos + BADGE_PADDING, IM_COL32(245, 245, 245, 240), text.c_str());
}

struct Shortcut {
    const char* name;
    Icon icon;
    const char* appName;
};

const Shortcut SHORTCUTS[] = {
    {"This PC", Icon::Computer, "File Explorer"},
    {"File Explorer", Icon::Folder, "File Explorer"},
    {"Terminal", Icon::Terminal, "Terminal"},
    {"Task Manager", Icon::TaskManager, "Task Manager"},
};

int selectedShortcut = -1;

void drawShortcuts(ImVec2 workMin, float top) {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec2 itemSize(74.0f, 78.0f);
    float startX = MARGIN + 8.0f;
    float startY = top + 42.0f;

    for (size_t i = 0; i < sizeof(SHORTCUTS) / sizeof(SHORTCUTS[0]); ++i) {
        const Shortcut& item = SHORTCUTS[i];
        ImVec2 pos(workMin.x + startX, workMin.y + startY + i * (itemSize.y + 10.0f));

        ImGui::SetCursorScreenPos(pos);
        ImGui::PushID(static_cast<int>(i));
        bool clicked = ImGui::InvisibleButton("##sc", itemSize);
        bool hovered = ImGui::IsItemHovered();
        bool doubleClicked = hovered && ImGui::IsMouseDoubleClicked(0);

        if (clicked) {
            selectedShortcut = static_cast<int>(i);
        }

        if (doubleClicked) {
            App* app = apps::find(item.appName);
            if (app) {
                apps::open(*app);
            }
        }

        if (hovered) {
            ImGui::SetTooltip("%s (Double-click to open)", item.name);
        }

        bool selected = (selectedShortcut == static_cast<int>(i));
        if (selected) {
            draw->AddRectFilled(pos, pos + itemSize, IM_COL32(0, 120, 215, 75), 6.0f);
            draw->AddRect(pos, pos + itemSize, IM_COL32(0, 120, 215, 180), 6.0f, 0, 1.0f);
        } else if (hovered) {
            draw->AddRectFilled(pos, pos + itemSize, IM_COL32(255, 255, 255, 30), 6.0f);
        }

        ImVec2 iconCenter = pos + ImVec2(itemSize.x * 0.5f, 26.0f);
        drawIcon(draw, item.icon, iconCenter, 36.0f);

        ImVec2 labelSize = ImGui::CalcTextSize(item.name);
        ImVec2 textPos = pos + ImVec2((itemSize.x - labelSize.x) * 0.5f, 48.0f);
        ImVec2 pillMin = textPos - ImVec2(5.0f, 1.0f);
        ImVec2 pillMax = textPos + labelSize + ImVec2(5.0f, 2.0f);

        if (!selected) {
            draw->AddRectFilled(pillMin, pillMax, IM_COL32(0, 0, 0, 130), 4.0f);
        }
        draw->AddText(textPos + ImVec2(1, 1), IM_COL32(0, 0, 0, 220), item.name);
        draw->AddText(textPos, IM_COL32(255, 255, 255, 245), item.name);

        ImGui::PopID();
    }
}

}

void init() {
    wallpaper = loadTexture(config.wallpaper);
}

void draw() {
    ImDrawList* bgDraw = ImGui::GetBackgroundDrawList();
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    if (screen.x <= 0 || screen.y <= 0) {
        return;
    }

    drawWallpaper(bgDraw, screen);
    float top = taskbar::workMin().y + MARGIN;
    drawBadge(bgDraw, ImVec2(MARGIN, top), config.osName);

    ImVec2 workMin = taskbar::workMin();
    ImVec2 workMax = taskbar::workMax();
    ImGui::SetNextWindowPos(workMin);
    ImGui::SetNextWindowSize(workMax - workMin);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("##desktop_layer", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::PopStyleVar(2);

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) {
        selectedShortcut = -1;
    }

    drawShortcuts(workMin, top);
    ImGui::End();
}

void shutdown() {
    freeTexture(wallpaper);
}

}
