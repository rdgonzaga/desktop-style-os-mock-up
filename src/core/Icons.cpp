#include "core/Icons.h"

namespace {

const ImU32 GREEN = IM_COL32(80, 230, 90, 255);

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

void drawTaskManager(ImDrawList* draw, ImVec2 c, float s) {
    auto p = [&](float x, float y) { return ImVec2(c.x + x * s, c.y + y * s); };
    ImU32 frame = IM_COL32(60, 60, 60, 255);
    draw->AddRectFilled(p(-3, 7), p(3, 11), frame);
    draw->AddRectFilled(p(-8, 11), p(8, 13), frame, 1 * s);
    draw->AddRectFilled(p(-14, -12), p(14, 8), frame, 2 * s);
    draw->AddRectFilled(p(-12, -10), p(12, 6), IM_COL32(0, 0, 0, 255));
    ImVec2 graph[] = {p(-11, 3), p(-6, -2), p(-2, 1), p(3, -7), p(7, -1), p(11, -4)};
    draw->AddPolyline(graph, 6, GREEN, 0, 1.5f * s);
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
    }
}
