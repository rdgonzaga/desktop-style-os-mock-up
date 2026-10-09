#include "shell/Taskbar.h"

#include "imgui.h"

#include "Config.h"
#include "core/Apps.h"
#include "core/Icons.h"
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
const ImU32 START_NORMAL = IM_COL32(60, 160, 60, 255);
const ImU32 START_HOVERED = IM_COL32(80, 185, 75, 255);
const ImU32 START_HELD = IM_COL32(45, 130, 45, 255);
const ImU32 PWR_NORMAL = IM_COL32(214, 72, 38, 255);
const ImU32 PWR_HOVERED = IM_COL32(235, 95, 55, 255);
const ImU32 PWR_HELD = IM_COL32(180, 45, 20, 255);
const ImU32 TASK_OPEN = IM_COL32(255, 255, 255, 35);
const ImU32 TASK_ACTIVE = IM_COL32(255, 255, 255, 80);
const ImU32 TASK_RUNNING = IM_COL32(150, 210, 255, 255);
const ImU32 MENU_HEADER = IM_COL32(0, 84, 227, 255);
const ImU32 MENU_FOOTER = IM_COL32(40, 100, 220, 255);
const ImU32 MENU_SELECTED = IM_COL32(49, 106, 197, 255);
const ImU32 WHITE = IM_COL32(255, 255, 255, 255);
const ImU32 BLACK = IM_COL32(0, 0, 0, 255);
const ImU32 HOVER = IM_COL32(255, 255, 255, 50);

const ImVec2 START_SIZE(104.0f, HEIGHT);
const ImVec2 TASK_SIZE(46.0f, 34.0f);
const ImVec2 ICON_SIZE(30.0f, 30.0f);
const ImVec2 PWR_SIZE(74.0f, 28.0f);
const float TRAY_PADDING = 8.0f;
const float MENU_WIDTH = 300.0f;
const float MENU_ROW = 42.0f;
const float PI = 3.14159265f;

const char* START_MENU = "start menu";
const char* SHUTDOWN_TITLE = "Turn off computer";

bool shutdownRequested = false;

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

void drawPowerSymbol(ImDrawList* draw, ImVec2 c, float radius) {
    draw->PathArcTo(c, radius, -PI / 2 + 0.7f, PI * 1.5f - 0.7f);
    draw->PathStroke(WHITE, 0, 2.0f);
    draw->AddLine(c + ImVec2(0, -radius - 1), c + ImVec2(0, -1), WHITE, 2.0f);
}

void startButton(ImVec2 pos, bool menuOpen) {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImGui::SetCursorScreenPos(pos);
    if (ImGui::InvisibleButton("##start", START_SIZE)) {
        ImGui::OpenPopup(START_MENU);
    }

    ImU32 fill = START_NORMAL;
    if (ImGui::IsItemActive() || menuOpen) {
        fill = START_HELD;
    } else if (ImGui::IsItemHovered()) {
        fill = START_HOVERED;
    }
    draw->AddRectFilled(pos, pos + START_SIZE, fill, 12.0f, ImDrawFlags_RoundCornersRight);

    ImVec2 grid = pos + ImVec2(16, 12);
    for (int row = 0; row < 2; row++) {
        for (int col = 0; col < 2; col++) {
            ImVec2 cell = grid + ImVec2(col * 9.0f, row * 9.0f);
            draw->AddRectFilled(cell, cell + ImVec2(7, 7), WHITE, 1.5f);
        }
    }

    // drawn twice, 1px apart, to fake a bold font
    ImVec2 textPos = pos + ImVec2(42, (HEIGHT - 22) / 2 - 1);
    draw->AddText(ImGui::GetFont(), 22.0f, textPos, WHITE, "start");
    draw->AddText(ImGui::GetFont(), 22.0f, textPos + ImVec2(1, 0), WHITE, "start");
}

void appButton(App& app, ImVec2 pos) {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImGui::SetCursorScreenPos(pos);
    ImGui::PushID(app.name);
    if (ImGui::InvisibleButton("##task", TASK_SIZE)) {
        apps::toggle(app);
    }
    ImGui::PopID();

    if (app.open) {
        bool inFront = !app.minimized && apps::isActive(app);
        draw->AddRectFilled(pos, pos + TASK_SIZE, inFront ? TASK_ACTIVE : TASK_OPEN, 4.0f);
        draw->AddRectFilled(ImVec2(pos.x + 8, pos.y + TASK_SIZE.y - 3), pos + TASK_SIZE - ImVec2(8, 0), TASK_RUNNING, 2.0f);
    }
    if (ImGui::IsItemHovered()) {
        draw->AddRectFilled(pos, pos + TASK_SIZE, HOVER, 4.0f);
        ImGui::SetTooltip("%s", app.name);
    }
    drawIcon(draw, app.icon, pos + TASK_SIZE * 0.5f, 26.0f);
}

void trayIcon(const char* id, ImVec2 pos, const char* tooltip, void (*drawGlyph)(ImDrawList*, ImVec2)) {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImGui::SetCursorScreenPos(pos);
    ImGui::InvisibleButton(id, ICON_SIZE);
    if (ImGui::IsItemHovered()) {
        draw->AddRectFilled(pos, pos + ICON_SIZE, HOVER, 4.0f);
        ImGui::SetTooltip("%s", tooltip);
    }
    drawGlyph(draw, pos + ICON_SIZE * 0.5f);
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

bool menuRow(const char* label, Icon icon) {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 size(MENU_WIDTH, MENU_ROW);
    bool clicked = ImGui::InvisibleButton(label, size);
    bool hovered = ImGui::IsItemHovered();

    if (hovered) {
        draw->AddRectFilled(pos + ImVec2(4, 0), pos + size - ImVec2(4, 0), MENU_SELECTED, 3.0f);
    }
    drawIcon(draw, icon, pos + ImVec2(28, MENU_ROW / 2), 28.0f);
    float textY = pos.y + (MENU_ROW - ImGui::GetTextLineHeight()) / 2;
    draw->AddText(ImVec2(pos.x + 54, textY), hovered ? WHITE : BLACK, label);
    return clicked;
}

void startMenu() {
    if (config.taskbarTop) {
        ImGui::SetNextWindowPos(ImVec2(0, HEIGHT));
    } else {
        ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - HEIGHT), ImGuiCond_Always, ImVec2(0, 1));
    }
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 6.0f);
    bool open = ImGui::BeginPopup(START_MENU);
    ImGui::PopStyleVar(2);

    if (open) {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 header = ImGui::GetCursorScreenPos();
        draw->AddRectFilled(header, header + ImVec2(MENU_WIDTH, 56), MENU_HEADER, 6.0f, ImDrawFlags_RoundCornersTop);
        draw->AddRectFilled(header + ImVec2(12, 10), header + ImVec2(48, 46), IM_COL32(245, 160, 60, 255), 4.0f);
        draw->AddRect(header + ImVec2(12, 10), header + ImVec2(48, 46), WHITE, 4.0f, 2.0f);
        draw->AddCircleFilled(header + ImVec2(30, 22), 6.0f, WHITE);
        draw->AddRectFilled(header + ImVec2(21, 31), header + ImVec2(39, 43), WHITE, 6.0f, ImDrawFlags_RoundCornersTop);
        draw->AddText(ImGui::GetFont(), 20.0f, header + ImVec2(60, 17), WHITE, "CSOPESY User");
        ImGui::Dummy(ImVec2(MENU_WIDTH, 62));

        for (App& app : apps::all()) {
            if (menuRow(app.name, app.icon)) {
                apps::open(app);
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::Dummy(ImVec2(0, 6));
        ImVec2 footer = ImGui::GetCursorScreenPos();
        draw->AddRectFilled(footer, footer + ImVec2(MENU_WIDTH, 44), MENU_FOOTER, 6.0f, ImDrawFlags_RoundCornersBottom);

        ImVec2 turnOff = footer + ImVec2(MENU_WIDTH - 176, 7);
        ImVec2 turnOffSize(168, 30);
        ImGui::SetCursorScreenPos(turnOff);
        if (ImGui::InvisibleButton("##turnoff", turnOffSize)) {
            shutdownRequested = true;
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsItemHovered()) {
            draw->AddRectFilled(turnOff, turnOff + turnOffSize, HOVER, 4.0f);
        }
        draw->AddRectFilled(turnOff + ImVec2(6, 5), turnOff + ImVec2(26, 25), PWR_NORMAL, 3.0f);
        drawPowerSymbol(draw, turnOff + ImVec2(16, 16), 5.0f);
        draw->AddText(turnOff + ImVec2(34, (turnOffSize.y - ImGui::GetTextLineHeight()) / 2), WHITE, "Turn Off Computer");

        ImGui::SetCursorScreenPos(footer);
        ImGui::Dummy(ImVec2(MENU_WIDTH, 44));
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();
}

void shutdownDialog() {
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (!theme::beginModal(SHUTDOWN_TITLE, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        return;
    }
    ImGui::Text("Are you sure you want to shut down %s?", config.osName.c_str());
    ImGui::Spacing();
    if (ImGui::Button("Turn Off", ImVec2(110, 0))) {
        power::state = power::State::Off;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(110, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

}

ImVec2 workMin() {
    return ImVec2(0, config.taskbarTop ? HEIGHT : 0);
}

ImVec2 workMax() {
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    return ImVec2(screen.x, config.taskbarTop ? screen.y : screen.y - HEIGHT);
}

void draw() {
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    ImVec2 barMin(0, config.taskbarTop ? 0 : screen.y - HEIGHT);
    ImVec2 barMax(screen.x, barMin.y + HEIGHT);

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
    float edge = config.taskbarTop ? barMax.y - 1 : barMin.y;
    draw->AddLine(ImVec2(0, edge), ImVec2(barMax.x, edge), BAR_HIGHLIGHT, 2.0f);

    startButton(barMin, ImGui::IsPopupOpen(START_MENU));

    float x = START_SIZE.x + 10;
    float taskY = barMin.y + (HEIGHT - TASK_SIZE.y) / 2 + 1;
    for (App& app : apps::all()) {
        appButton(app, ImVec2(x, taskY));
        x += TASK_SIZE.x + 4;
    }

    float trayWidth = TRAY_PADDING * 3 + ICON_SIZE.x * 2 + PWR_SIZE.x;
    ImVec2 trayMin(barMax.x - trayWidth, barMin.y + 2);
    draw->AddRectFilledMultiColor(trayMin, barMax, TRAY_TOP, TRAY_TOP, TRAY_BOTTOM, TRAY_BOTTOM);
    draw->AddLine(trayMin, ImVec2(trayMin.x, barMax.y), TRAY_EDGE, 1.0f);

    float iconY = barMin.y + (HEIGHT - ICON_SIZE.y) / 2 + 1;
    float trayX = trayMin.x + TRAY_PADDING;
    trayIcon("##vol", ImVec2(trayX, iconY), "Volume: 75%", drawSpeaker);
    trayX += ICON_SIZE.x;
    trayIcon("##net", ImVec2(trayX, iconY), "Network: Connected", drawSignalBars);
    trayX += ICON_SIZE.x + TRAY_PADDING;
    powerButton(ImVec2(trayX, barMin.y + (HEIGHT - PWR_SIZE.y) / 2 + 1));

    startMenu();
    if (shutdownRequested) {
        ImGui::OpenPopup(SHUTDOWN_TITLE);
        shutdownRequested = false;
    }
    shutdownDialog();
    ImGui::End();
}

}
