#include "shell/Taskbar.h"

#include "imgui.h"

#include "Config.h"
#include "core/Power.h"
#include "core/Theme.h"

namespace taskbar {

namespace {

const ImU32 BAR_TOP = IM_COL32(45, 110, 235, 255);
const ImU32 BAR_BOTTOM = IM_COL32(28, 70, 190, 255);
const ImU32 BAR_HIGHLIGHT = IM_COL32(110, 165, 255, 255);
const ImU32 TRAY_TOP = IM_COL32(20, 145, 240, 255);
const ImU32 TRAY_BOTTOM = IM_COL32(15, 110, 210, 255);
const ImU32 TRAY_EDGE = IM_COL32(16, 66, 175, 255);
const ImU32 PWR_NORMAL = IM_COL32(214, 72, 38, 255);
const ImU32 PWR_HOVERED = IM_COL32(235, 95, 55, 255);
const ImU32 PWR_HELD = IM_COL32(180, 45, 20, 255);
const ImU32 WHITE = IM_COL32(255, 255, 255, 255);
const ImU32 HOVER = IM_COL32(255, 255, 255, 50);

const float TRAY_PADDING = 8.0f;
const ImVec2 ICON_SIZE(30.0f, 30.0f);
const ImVec2 PWR_SIZE(74.0f, 28.0f);
const float PI = 3.14159265f;

const char* SHUTDOWN_TITLE = "Turn off computer";

void drawSpeaker(ImDrawList* draw, ImVec2 c) {
    draw->AddRectFilled(c + ImVec2(-8, -3), c + ImVec2(-4, 3), WHITE);
    draw->AddQuadFilled(c + ImVec2(-4, -3), c + ImVec2(1, -7), c + ImVec2(1, 7), c + ImVec2(-4, 3), WHITE);
    draw->PathArcTo(c + ImVec2(1, 0), 5.0f, -0.9f, 0.9f);
    draw->PathStroke(WHITE, 0, 1.5f);
    draw->PathArcTo(c + ImVec2(1, 0), 9.0f, -0.9f, 0.9f);
    draw->PathStroke(WHITE, 0, 1.5f);
}

void drawSignalBars(ImDrawList* draw, ImVec2 c) {
    for (int i = 0; i < 4; i++) {
        float x = c.x - 8 + i * 4.5f;
        float height = 4.0f + i * 3.5f;
        draw->AddRectFilled(ImVec2(x, c.y + 7 - height), ImVec2(x + 3, c.y + 7), WHITE);
    }
}

// a circle open at the top with a line through the gap
void drawPowerSymbol(ImDrawList* draw, ImVec2 c, float radius) {
    draw->PathArcTo(c, radius, -PI / 2 + 0.7f, PI * 1.5f - 0.7f);
    draw->PathStroke(WHITE, 0, 2.0f);
    draw->AddLine(c + ImVec2(0, -radius - 1), c + ImVec2(0, -1), WHITE, 2.0f);
}

void trayIcon(const char* id, ImVec2 pos, const char* tooltip, void (*drawIcon)(ImDrawList*, ImVec2)) {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImGui::SetCursorScreenPos(pos);
    ImGui::InvisibleButton(id, ICON_SIZE);
    if (ImGui::IsItemHovered()) {
        draw->AddRectFilled(pos, pos + ICON_SIZE, HOVER, 4.0f);
        ImGui::SetTooltip("%s", tooltip);
    }
    drawIcon(draw, pos + ICON_SIZE * 0.5f);
}

void powerButton(ImVec2 pos) {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImGui::SetCursorScreenPos(pos);
    if (ImGui::InvisibleButton("##pwr", PWR_SIZE)) {
        ImGui::OpenPopup(SHUTDOWN_TITLE);
    }

    ImU32 fill = ImGui::IsItemActive() ? PWR_HELD : ImGui::IsItemHovered() ? PWR_HOVERED : PWR_NORMAL;
    draw->AddRectFilled(pos, pos + PWR_SIZE, fill, 4.0f);
    draw->AddRect(pos, pos + PWR_SIZE, IM_COL32(255, 255, 255, 120), 4.0f);
    drawPowerSymbol(draw, pos + ImVec2(17, PWR_SIZE.y / 2 + 1), 6.0f);
    ImVec2 textSize = ImGui::CalcTextSize("PWR");
    draw->AddText(pos + ImVec2(30, (PWR_SIZE.y - textSize.y) / 2), WHITE, "PWR");
}

void shutdownDialog() {
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (!theme::beginModal(SHUTDOWN_TITLE, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        return;
    }
    ImGui::Text("Are you sure you want to shut down %s?", config.osName.c_str());
    ImGui::Spacing();
    if (ImGui::Button("Turn Off", ImVec2(110, 0))) {
        power::on = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(110, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

}

void draw() {
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    ImVec2 barMin(0, screen.y - HEIGHT);
    ImVec2 barMax(screen.x, screen.y);

    ImGui::SetNextWindowPos(barMin);
    ImGui::SetNextWindowSize(barMax - barMin);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("##taskbar", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing);
    ImGui::PopStyleVar(3);

    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilledMultiColor(barMin, barMax, BAR_TOP, BAR_TOP, BAR_BOTTOM, BAR_BOTTOM);
    draw->AddLine(barMin, ImVec2(barMax.x, barMin.y), BAR_HIGHLIGHT, 2.0f);

    float trayWidth = TRAY_PADDING * 3 + ICON_SIZE.x * 2 + PWR_SIZE.x;
    ImVec2 trayMin(barMax.x - trayWidth, barMin.y + 2);
    draw->AddRectFilledMultiColor(trayMin, barMax, TRAY_TOP, TRAY_TOP, TRAY_BOTTOM, TRAY_BOTTOM);
    draw->AddLine(trayMin, ImVec2(trayMin.x, barMax.y), TRAY_EDGE, 1.0f);

    float iconY = barMin.y + (HEIGHT - ICON_SIZE.y) / 2 + 1;
    float x = trayMin.x + TRAY_PADDING;
    trayIcon("##vol", ImVec2(x, iconY), "Volume: 75%", drawSpeaker);
    x += ICON_SIZE.x;
    trayIcon("##net", ImVec2(x, iconY), "Network: Connected", drawSignalBars);
    x += ICON_SIZE.x + TRAY_PADDING;
    powerButton(ImVec2(x, barMin.y + (HEIGHT - PWR_SIZE.y) / 2 + 1));

    shutdownDialog();
    ImGui::End();
}

}
