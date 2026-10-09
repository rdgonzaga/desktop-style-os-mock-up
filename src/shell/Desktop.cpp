#include "shell/Desktop.h"

#include <string>

#include "imgui.h"

#include "Config.h"
#include "core/Clock.h"
#include "core/Texture.h"

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
    draw->AddRectFilled(pos, pos + badgeSize(text), IM_COL32(0, 0, 0, 150), 6.0f);
    draw->AddText(pos + BADGE_PADDING, IM_COL32(255, 255, 255, 235), text.c_str());
}

}

void init() {
    wallpaper = loadTexture(config.wallpaper);
}

void draw() {
    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    if (screen.x <= 0 || screen.y <= 0) {
        return;
    }

    drawWallpaper(draw, screen);
    drawBadge(draw, ImVec2(MARGIN, MARGIN), config.osName);

    std::string clock = sysclock::dateTime();
    drawBadge(draw, ImVec2(screen.x - badgeSize(clock).x - MARGIN, MARGIN), clock);
}

void shutdown() {
    freeTexture(wallpaper);
}

}
