#include "core/Icons.h"

namespace {

const ImU32 GREEN = IM_COL32(80, 230, 90, 255);
const ImU32 WHITE = IM_COL32(255, 255, 255, 255);
const ImU32 OUTLINE = IM_COL32(110, 110, 110, 255);

void drawTerminal(ImDrawList* draw, ImVec2 c, float s) {
    auto p = [&](float x, float y) { return ImVec2(c.x + x * s, c.y + y * s); };
    draw->AddRectFilled(p(-14, -12), p(14, 12), IM_COL32(20, 20, 20, 255), 3 * s);
    draw->AddRectFilled(p(-14, -12), p(14, -7), IM_COL32(90, 90, 90, 255), 3 * s, ImDrawFlags_RoundCornersTop);
    draw->AddRect(p(-14, -12), p(14, 12), IM_COL32(170, 170, 170, 255), 3 * s, 0, 1.2f * s);
    ImVec2 prompt[] = {p(-9, -3), p(-4, 1), p(-9, 5)};
    draw->AddPolyline(prompt, 3, GREEN, 0, 2.0f * s);
    draw->AddLine(p(-1, 6), p(7, 6), GREEN, 2.0f * s);
}

void drawFolder(ImDrawList* draw, ImVec2 c, float s) {
    auto p = [&](float x, float y) { return ImVec2(c.x + x * s, c.y + y * s); };
    ImU32 back = IM_COL32(214, 160, 40, 255);
    draw->AddRectFilled(p(-14, -11), p(-3, -5), back, 2 * s, ImDrawFlags_RoundCornersTop);
    draw->AddRectFilled(p(-14, -7), p(14, 11), back, 2 * s);
    draw->AddRectFilled(p(-14, -3), p(14, 11), IM_COL32(252, 212, 90, 255), 2 * s);
}

void drawMonitor(ImDrawList* draw, ImVec2 c, float s) {
    auto p = [&](float x, float y) { return ImVec2(c.x + x * s, c.y + y * s); };
    ImU32 frame = IM_COL32(60, 60, 60, 255);
    draw->AddRectFilled(p(-3, 7), p(3, 11), frame);
    draw->AddRectFilled(p(-8, 11), p(8, 13), frame, 1 * s);
    draw->AddRectFilled(p(-14, -12), p(14, 8), frame, 2 * s);
}

void drawTaskManager(ImDrawList* draw, ImVec2 c, float s) {
    auto p = [&](float x, float y) { return ImVec2(c.x + x * s, c.y + y * s); };
    drawMonitor(draw, c, s);
    draw->AddRectFilled(p(-12, -10), p(12, 6), IM_COL32(0, 0, 0, 255));
    ImVec2 graph[] = {p(-11, 3), p(-6, -2), p(-2, 1), p(3, -7), p(7, -1), p(11, -4)};
    draw->AddPolyline(graph, 6, GREEN, 0, 1.5f * s);
}

void drawComputer(ImDrawList* draw, ImVec2 c, float s) {
    auto p = [&](float x, float y) { return ImVec2(c.x + x * s, c.y + y * s); };
    drawMonitor(draw, c, s);
    ImU32 sky = IM_COL32(60, 120, 220, 255);
    ImU32 grass = IM_COL32(70, 150, 50, 255);
    draw->AddRectFilledMultiColor(p(-12, -10), p(12, 6), sky, sky, grass, grass);
}

void drawWindow(ImDrawList* draw, ImVec2 c, float s) {
    auto p = [&](float x, float y) { return ImVec2(c.x + x * s, c.y + y * s); };
    draw->AddRectFilled(p(-13, -11), p(13, 11), WHITE, 2 * s);
    draw->AddRectFilled(p(-13, -11), p(13, -5), IM_COL32(0, 84, 227, 255), 2 * s, ImDrawFlags_RoundCornersTop);
    draw->AddRect(p(-13, -11), p(13, 11), IM_COL32(0, 60, 116, 255), 2 * s, 0, 1.2f * s);
}

void drawFile(ImDrawList* draw, ImVec2 c, float s) {
    auto p = [&](float x, float y) { return ImVec2(c.x + x * s, c.y + y * s); };
    ImVec2 page[] = {p(-9, -13), p(4, -13), p(10, -7), p(10, 13), p(-9, 13)};
    draw->AddConvexPolyFilled(page, 5, WHITE);
    draw->AddPolyline(page, 5, OUTLINE, ImDrawFlags_Closed, 1.2f * s);
    ImVec2 fold[] = {p(4, -13), p(4, -7), p(10, -7)};
    draw->AddPolyline(fold, 3, OUTLINE, 0, 1.2f * s);
    for (int i = 0; i < 4; i++) {
        float y = -2.0f + i * 4.0f;
        draw->AddLine(p(-5, y), p(6, y), IM_COL32(120, 150, 210, 255), 1.2f * s);
    }
}

void drawDrive(ImDrawList* draw, ImVec2 c, float s) {
    auto p = [&](float x, float y) { return ImVec2(c.x + x * s, c.y + y * s); };
    draw->AddRectFilled(p(-14, -7), p(14, 8), IM_COL32(205, 205, 205, 255), 2 * s);
    draw->AddRect(p(-14, -7), p(14, 8), OUTLINE, 2 * s, 0, 1.2f * s);
    draw->AddLine(p(-14, 3), p(14, 3), OUTLINE, 1.0f * s);
    draw->AddRectFilled(p(-11, 4), p(-6, 7), IM_COL32(60, 180, 75, 255));
}

}

void drawIcon(ImDrawList* draw, Icon icon, ImVec2 center, float size) {
    float scale = size / 32.0f;
    switch (icon) {
    case Icon::Terminal:
        drawTerminal(draw, center, scale);
        break;
    case Icon::Folder:
        drawFolder(draw, center, scale);
        break;
    case Icon::TaskManager:
        drawTaskManager(draw, center, scale);
        break;
    case Icon::Window:
        drawWindow(draw, center, scale);
        break;
    case Icon::File:
        drawFile(draw, center, scale);
        break;
    case Icon::Drive:
        drawDrive(draw, center, scale);
        break;
    case Icon::Computer:
        drawComputer(draw, center, scale);
        break;
    }
}
