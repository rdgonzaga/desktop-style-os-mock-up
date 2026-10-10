#include "shell/Taskbar.h"

#include <cctype>
#include <string>

#include "imgui.h"

#include "Config.h"
#include "core/Apps.h"
#include "core/Clock.h"
#include "core/Icons.h"
#include "core/Power.h"
#include "core/Theme.h"

namespace taskbar {

namespace {

const ImU32 BAR_TOP = IM_COL32(30, 30, 30, 245);
const ImU32 BAR_BOTTOM = IM_COL32(20, 20, 20, 252);
const ImU32 BAR_HIGHLIGHT = IM_COL32(255, 255, 255, 25);
const ImU32 TRAY_TOP = IM_COL32(30, 30, 30, 0);
const ImU32 TRAY_BOTTOM = IM_COL32(20, 20, 20, 0);
const ImU32 TRAY_EDGE = IM_COL32(255, 255, 255, 18);
const ImU32 START_NORMAL = IM_COL32(255, 255, 255, 0);
const ImU32 START_HOVERED = IM_COL32(255, 255, 255, 25);
const ImU32 START_HELD = IM_COL32(255, 255, 255, 45);
const ImU32 PWR_NORMAL = IM_COL32(195, 48, 48, 255);
const ImU32 PWR_HOVERED = IM_COL32(225, 65, 65, 255);
const ImU32 PWR_HELD = IM_COL32(155, 35, 35, 255);
const ImU32 TASK_OPEN = IM_COL32(255, 255, 255, 20);
const ImU32 TASK_ACTIVE = IM_COL32(255, 255, 255, 45);
const ImU32 TASK_RUNNING = IM_COL32(0, 120, 215, 255);
const ImU32 MENU_HEADER = IM_COL32(28, 28, 28, 255);
const ImU32 MENU_FOOTER = IM_COL32(24, 24, 24, 255);
const ImU32 MENU_SELECTED = IM_COL32(255, 255, 255, 25);
const ImU32 WHITE = IM_COL32(255, 255, 255, 255);
const ImU32 HOVER = IM_COL32(255, 255, 255, 28);

const ImVec2 START_SIZE(42.0f, 34.0f);
const ImVec2 TASK_SIZE(46.0f, 34.0f);
const ImVec2 ICON_SIZE(30.0f, 30.0f);
const ImVec2 PWR_SIZE(74.0f, 28.0f);
const float TRAY_PADDING = 8.0f;
const float MENU_WIDTH = 340.0f;
const float MENU_ROW = 42.0f;
const float PI = 3.14159265f;

const char* START_MENU = "start menu";
const char* SHUTDOWN_TITLE = "Turn off computer";

bool shutdownRequested = false;

int volumeLevel = 75;
int brightnessLevel = 85;
bool wifiEnabled = true;
bool bluetoothEnabled = true;
bool nightLight = false;

void drawSpeaker(ImDrawList* draw, ImVec2 c) {
    draw->AddRectFilled(c + ImVec2(-8, -3), c + ImVec2(-4, 3), WHITE);
    draw->AddQuadFilled(c + ImVec2(-4, -3), c + ImVec2(1, -7), c + ImVec2(1, 7), c + ImVec2(-4, 3), WHITE);
    if (volumeLevel == 0) {
        draw->AddLine(c + ImVec2(3, -4), c + ImVec2(9, 4), IM_COL32(235, 60, 60, 255), 2.0f);
        draw->AddLine(c + ImVec2(3, 4), c + ImVec2(9, -4), IM_COL32(235, 60, 60, 255), 2.0f);
        return;
    }
    draw->PathArcTo(c + ImVec2(1, 0), 5.0f, -0.9f, 0.9f);
    draw->PathStroke(WHITE, 0, 1.5f);
    if (volumeLevel > 40) {
        draw->PathArcTo(c + ImVec2(1, 0), 9.0f, -0.9f, 0.9f);
        draw->PathStroke(WHITE, 0, 1.5f);
    }
}

void drawSignalBars(ImDrawList* draw, ImVec2 c) {
    if (!wifiEnabled) {
        for (int i = 0; i < 4; i++) {
            float x = c.x - 8 + i * 4.5f;
            float height = 4.0f + i * 3.5f;
            draw->AddRectFilled(ImVec2(x, c.y + 7 - height), ImVec2(x + 3, c.y + 7), IM_COL32(110, 110, 110, 180));
        }
        draw->AddLine(c - ImVec2(6, 6), c + ImVec2(6, 6), IM_COL32(235, 60, 60, 255), 2.0f);
        draw->AddLine(c + ImVec2(-6, 6), c + ImVec2(6, -6), IM_COL32(235, 60, 60, 255), 2.0f);
        return;
    }
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
        ImGui::SetTooltip("Start");
    }
    draw->AddRectFilled(pos, pos + START_SIZE, fill, 6.0f);

    ImVec2 grid = pos + ImVec2((START_SIZE.x - 17.0f) * 0.5f, (START_SIZE.y - 17.0f) * 0.5f);
    ImU32 winLogo = (ImGui::IsItemHovered() || menuOpen) ? IM_COL32(75, 175, 255, 255) : IM_COL32(0, 120, 215, 255);
    for (int row = 0; row < 2; row++) {
        for (int col = 0; col < 2; col++) {
            ImVec2 cell = grid + ImVec2(col * 9.5f, row * 9.5f);
            draw->AddRectFilled(cell, cell + ImVec2(7.5f, 7.5f), winLogo, 1.5f);
        }
    }
}

void appButton(App& app, ImVec2 pos) {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImGui::SetCursorScreenPos(pos);
    ImGui::PushID(app.name);
    if (ImGui::InvisibleButton("##task", TASK_SIZE)) {
        apps::toggle(app);
    }
    ImGui::PopID();

    bool inFront = app.open && !app.minimized && apps::isActive(app);
    if (app.open) {
        draw->AddRectFilled(pos, pos + TASK_SIZE, inFront ? TASK_ACTIVE : TASK_OPEN, 6.0f);
        float pillMargin = inFront ? 10.0f : 15.0f;
        draw->AddRectFilled(ImVec2(pos.x + pillMargin, pos.y + TASK_SIZE.y - 3),
                            ImVec2(pos.x + TASK_SIZE.x - pillMargin, pos.y + TASK_SIZE.y), TASK_RUNNING, 2.0f);
    }
    if (ImGui::IsItemHovered()) {
        draw->AddRectFilled(pos, pos + TASK_SIZE, HOVER, 6.0f);
        ImGui::SetTooltip("%s", app.name);
    }
    drawIcon(draw, app.icon, pos + TASK_SIZE * 0.5f, 26.0f);
}

bool trayIcon(const char* id, ImVec2 pos, const char* tooltip, void (*drawGlyph)(ImDrawList*, ImVec2)) {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImGui::SetCursorScreenPos(pos);
    bool clicked = ImGui::InvisibleButton(id, ICON_SIZE);
    if (ImGui::IsItemHovered()) {
        draw->AddRectFilled(pos, pos + ICON_SIZE, HOVER, 6.0f);
        ImGui::SetTooltip("%s", tooltip);
    }
    drawGlyph(draw, pos + ICON_SIZE * 0.5f);
    return clicked;
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
        draw->AddRectFilled(pos + ImVec2(6, 0), pos + size - ImVec2(6, 0), MENU_SELECTED, 4.0f);
    }
    drawIcon(draw, icon, pos + ImVec2(28, MENU_ROW / 2), 26.0f);
    float textY = pos.y + (MENU_ROW - ImGui::GetTextLineHeight()) / 2;
    draw->AddText(ImVec2(pos.x + 54, textY), WHITE, label);
    return clicked;
}

void startMenu(float startX) {
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    float menuX = config.taskbarCentered ? (startX + (START_SIZE.x - MENU_WIDTH) * 0.5f) : 0.0f;
    if (menuX < 10.0f) menuX = 10.0f;
    if (menuX + MENU_WIDTH > screen.x - 10.0f) menuX = screen.x - MENU_WIDTH - 10.0f;

    if (config.taskbarTop) {
        ImGui::SetNextWindowPos(ImVec2(menuX, HEIGHT + 4));
    } else {
        ImGui::SetNextWindowPos(ImVec2(menuX, screen.y - HEIGHT - 4), ImGuiCond_Always, ImVec2(0, 1));
    }
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 10.0f);
    bool open = ImGui::BeginPopup(START_MENU);
    ImGui::PopStyleVar(2);

    static char searchBuf[64] = "";
    static bool wasOpen = false;

    if (open) {
        if (!wasOpen) {
            searchBuf[0] = '\0';
            wasOpen = true;
        }

        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 header = ImGui::GetCursorScreenPos();
        draw->AddRectFilled(header, header + ImVec2(MENU_WIDTH, 56), MENU_HEADER, 10.0f, ImDrawFlags_RoundCornersTop);
        ImVec2 avatarCenter = header + ImVec2(32, 28);
        draw->AddCircleFilled(avatarCenter, 16.0f, IM_COL32(0, 120, 215, 255));
        draw->AddCircleFilled(avatarCenter - ImVec2(0, 3), 5.5f, WHITE);
        draw->AddRectFilled(avatarCenter + ImVec2(-7, 3), avatarCenter + ImVec2(7, 11), WHITE, 4.0f, ImDrawFlags_RoundCornersTop);
        draw->AddText(header + ImVec2(58, (56 - ImGui::GetTextLineHeight()) / 2), WHITE, "CSOPESY User");

        // Modern search bar
        ImGui::SetCursorScreenPos(header + ImVec2(10, 62));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 6));
        ImGui::PushItemWidth(MENU_WIDTH - 20);
        bool enterPressed = ImGui::InputTextWithHint("##menu_search", "Type here to search...", searchBuf, sizeof(searchBuf), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::PopItemWidth();
        ImGui::PopStyleVar(2);

        ImGui::Dummy(ImVec2(MENU_WIDTH, 4));

        std::string query = searchBuf;
        for (char& c : query) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

        int matchCount = 0;
        App* firstMatch = nullptr;
        for (App& app : apps::all()) {
            std::string appName = app.name;
            for (char& c : appName) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

            if (query.empty() || appName.find(query) != std::string::npos) {
                matchCount++;
                if (!firstMatch) firstMatch = &app;
                if (menuRow(app.name, app.icon)) {
                    apps::open(app);
                    ImGui::CloseCurrentPopup();
                }
            }
        }

        if (enterPressed && firstMatch) {
            apps::open(*firstMatch);
            ImGui::CloseCurrentPopup();
        }

        if (matchCount == 0) {
            ImGui::Dummy(ImVec2(0, 6));
            ImGui::SetCursorPosX(16);
            ImGui::TextDisabled("No apps match \"%s\"", searchBuf);
            ImGui::Dummy(ImVec2(0, 6));
        }

        ImGui::Dummy(ImVec2(0, 6));
        ImVec2 footer = ImGui::GetCursorScreenPos();
        draw->AddRectFilled(footer, footer + ImVec2(MENU_WIDTH, 44), MENU_FOOTER, 10.0f, ImDrawFlags_RoundCornersBottom);
        draw->AddLine(footer, footer + ImVec2(MENU_WIDTH, 0), IM_COL32(255, 255, 255, 18), 1.0f);

        ImVec2 labelSize = ImGui::CalcTextSize("Turn Off Computer");
        float turnOffWidth = 36.0f + labelSize.x + 14.0f;
        ImVec2 turnOff = footer + ImVec2(MENU_WIDTH - turnOffWidth - 10, 7);
        ImVec2 turnOffSize(turnOffWidth, 30);
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
    } else {
        wasOpen = false;
    }
    ImGui::PopStyleVar();
}

const char* QUICK_SETTINGS = "quick settings";

void quickSettings() {
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    const float QUICK_WIDTH = 320.0f;
    if (config.taskbarTop) {
        ImGui::SetNextWindowPos(ImVec2(screen.x - QUICK_WIDTH - 12, HEIGHT + 6));
    } else {
        ImGui::SetNextWindowPos(ImVec2(screen.x - QUICK_WIDTH - 12, screen.y - HEIGHT - 6), ImGuiCond_Always, ImVec2(0, 1));
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 12.0f);
    if (ImGui::BeginPopup(QUICK_SETTINGS)) {
        ImDrawList* draw = ImGui::GetWindowDrawList();

        ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.95f, 1.0f), "Quick Settings");
        ImGui::Separator();
        ImGui::Spacing();

        auto drawToggle = [&](const char* label, const char* subtext, bool& state, const char* id, float customWidth = 139.0f) {
            ImVec2 size(customWidth, 48.0f);
            ImVec2 pos = ImGui::GetCursorScreenPos();
            if (ImGui::InvisibleButton(id, size)) {
                state = !state;
            }
            bool hovered = ImGui::IsItemHovered();
            ImU32 bg = state ? (hovered ? IM_COL32(0, 140, 240, 255) : IM_COL32(0, 120, 215, 255))
                             : (hovered ? IM_COL32(52, 52, 52, 255) : IM_COL32(40, 40, 40, 255));
            draw->AddRectFilled(pos, pos + size, bg, 8.0f);
            draw->AddRect(pos, pos + size, state ? IM_COL32(255, 255, 255, 60) : IM_COL32(255, 255, 255, 22), 8.0f);

            draw->AddText(pos + ImVec2(12, 8), WHITE, label);
            draw->AddText(pos + ImVec2(12, 26), state ? IM_COL32(220, 240, 255, 220) : IM_COL32(160, 160, 160, 220), subtext);
        };

        drawToggle("Wi-Fi", wifiEnabled ? "Connected" : "Off", wifiEnabled, "##wifi_toggle", 139.0f);
        ImGui::SameLine(0, 10);
        drawToggle("Bluetooth", bluetoothEnabled ? "On" : "Off", bluetoothEnabled, "##bt_toggle", 139.0f);

        ImGui::Spacing();

        drawToggle("Night light", nightLight ? "On (Warm amber display)" : "Off", nightLight, "##nl_toggle", QUICK_WIDTH - 32.0f);

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::Text("Volume: %d%%", volumeLevel);
        ImGui::PushItemWidth(QUICK_WIDTH - 32);
        ImGui::SliderInt("##vol_slider", &volumeLevel, 0, 100, "%d%%");
        ImGui::PopItemWidth();

        ImGui::Spacing();

        ImGui::Text("Brightness: %d%%", brightnessLevel);
        ImGui::PushItemWidth(QUICK_WIDTH - 32);
        ImGui::SliderInt("##bright_slider", &brightnessLevel, 0, 100, "%d%%");
        ImGui::PopItemWidth();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextColored(ImVec4(0.55f, 0.85f, 1.0f, 1.0f), "Battery: 100%% (Fully Charged)");

        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(2);
}

const char* VOLUME_POPUP = "volume popup";
const char* CALENDAR_POPUP = "calendar flyout";

void volumeFlyout() {
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    const float VOL_WIDTH = 280.0f;
    if (config.taskbarTop) {
        ImGui::SetNextWindowPos(ImVec2(screen.x - VOL_WIDTH - 12, HEIGHT + 6));
    } else {
        ImGui::SetNextWindowPos(ImVec2(screen.x - VOL_WIDTH - 12, screen.y - HEIGHT - 6), ImGuiCond_Always, ImVec2(0, 1));
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 14));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 12.0f);
    if (ImGui::BeginPopup(VOLUME_POPUP)) {
        ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.95f, 1.0f), "Volume: %d%%", volumeLevel);
        ImGui::Spacing();
        ImGui::PushItemWidth(VOL_WIDTH - 32);
        ImGui::SliderInt("##vol_slider_only", &volumeLevel, 0, 100, "%d%%");
        ImGui::PopItemWidth();
        ImGui::Spacing();
        if (ImGui::Button(volumeLevel == 0 ? "Unmute" : "Mute", ImVec2(VOL_WIDTH - 32, 28))) {
            static int prevVolume = 75;
            if (volumeLevel > 0) {
                prevVolume = volumeLevel;
                volumeLevel = 0;
            } else {
                volumeLevel = prevVolume > 0 ? prevVolume : 75;
            }
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(2);
}

void calendarFlyout() {
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    const float CAL_WIDTH = 260.0f;
    if (config.taskbarTop) {
        ImGui::SetNextWindowPos(ImVec2(screen.x - CAL_WIDTH - 12, HEIGHT + 6));
    } else {
        ImGui::SetNextWindowPos(ImVec2(screen.x - CAL_WIDTH - 12, screen.y - HEIGHT - 6), ImGuiCond_Always, ImVec2(0, 1));
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 14));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 12.0f);
    if (ImGui::BeginPopup(CALENDAR_POPUP)) {
        ImGui::TextColored(ImVec4(0.0f, 0.55f, 0.95f, 1.0f), "%s", sysclock::dateStr().c_str());
        ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.95f, 1.0f), "%s", sysclock::timeStr().c_str());
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextUnformatted(sysclock::dateTime().c_str());
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(2);
}

void trayClock(ImVec2 pos) {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 clockSize(76.0f, 32.0f);
    ImGui::SetCursorScreenPos(pos);
    if (ImGui::InvisibleButton("##tray_clock", clockSize)) {
        ImGui::OpenPopup(CALENDAR_POPUP);
    }
    bool hovered = ImGui::IsItemHovered();
    if (hovered) {
        draw->AddRectFilled(pos, pos + clockSize, HOVER, 4.0f);
        ImGui::SetTooltip("%s", sysclock::dateTime().c_str());
    }

    std::string timeText = sysclock::timeStr();
    std::string dateText = sysclock::dateStr();

    ImVec2 timeSize = ImGui::CalcTextSize(timeText.c_str());
    ImVec2 dateSize = ImGui::CalcTextSize(dateText.c_str());

    float timeX = pos.x + (clockSize.x - timeSize.x) * 0.5f;
    float dateX = pos.x + (clockSize.x - dateSize.x) * 0.5f;

    draw->AddText(ImVec2(timeX, pos.y + 1.0f), WHITE, timeText.c_str());
    draw->AddText(ImVec2(dateX, pos.y + 15.0f), IM_COL32(185, 185, 185, 230), dateText.c_str());
}

void shutdownDialog() {
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (!theme::beginModal(SHUTDOWN_TITLE, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        return;
    }
    ImGui::Text("Are you sure you want to shut down %s?", config.osName.c_str());
    ImGui::Spacing();
    if (ImGui::Button("Turn Off", ImVec2(110, 0))) {
        power::state = power::State::ShuttingDown;
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

    float totalAppsWidth = static_cast<float>(apps::all().size()) * (TASK_SIZE.x + 4.0f);
    float groupWidth = START_SIZE.x + 8.0f + totalAppsWidth;

    float startX = 0.0f;
    if (config.taskbarCentered) {
        startX = (screen.x - groupWidth) * 0.5f;
        if (startX < 0.0f) startX = 0.0f;
    }

    startButton(ImVec2(startX, barMin.y), ImGui::IsPopupOpen(START_MENU));

    float x = startX + START_SIZE.x + 8.0f;
    float taskY = barMin.y + (HEIGHT - TASK_SIZE.y) / 2 + 1;
    for (App& app : apps::all()) {
        appButton(app, ImVec2(x, taskY));
        x += TASK_SIZE.x + 4;
    }

    float clockWidth = 76.0f;
    float trayWidth = TRAY_PADDING * 4 + ICON_SIZE.x * 2 + clockWidth + PWR_SIZE.x;
    ImVec2 trayMin(barMax.x - trayWidth, barMin.y + 2);
    draw->AddRectFilledMultiColor(trayMin, barMax, TRAY_TOP, TRAY_TOP, TRAY_BOTTOM, TRAY_BOTTOM);
    draw->AddLine(trayMin, ImVec2(trayMin.x, barMax.y), TRAY_EDGE, 1.0f);

    float iconY = barMin.y + (HEIGHT - ICON_SIZE.y) / 2 + 1;
    float trayX = trayMin.x + TRAY_PADDING;
    std::string volTooltip = volumeLevel == 0 ? "Speakers: Muted\n(Click for Volume)" : ("Speakers: " + std::to_string(volumeLevel) + "%\n(Click for Volume)");
    if (trayIcon("##vol", ImVec2(trayX, iconY), volTooltip.c_str(), drawSpeaker)) {
        ImGui::OpenPopup(VOLUME_POPUP);
    }
    trayX += ICON_SIZE.x;
    std::string netTooltip = wifiEnabled ? "Wi-Fi: Connected (CSOPESY-5G)\n(Click for Quick Settings)" : "Wi-Fi: Disconnected\n(Click for Quick Settings)";
    if (trayIcon("##net", ImVec2(trayX, iconY), netTooltip.c_str(), drawSignalBars)) {
        ImGui::OpenPopup(QUICK_SETTINGS);
    }
    trayX += ICON_SIZE.x + TRAY_PADDING;

    float clockY = barMin.y + (HEIGHT - 32.0f) / 2 + 1;
    trayClock(ImVec2(trayX, clockY));
    trayX += clockWidth + TRAY_PADDING;

    powerButton(ImVec2(trayX, barMin.y + (HEIGHT - PWR_SIZE.y) / 2 + 1));

    startMenu(startX);
    volumeFlyout();
    quickSettings();
    calendarFlyout();

    if (shutdownRequested) {
        ImGui::OpenPopup(SHUTDOWN_TITLE);
        shutdownRequested = false;
    }
    shutdownDialog();
    ImGui::End();
}

static float volumeOsdTimer = 0.0f;
static int lastVolume = 75;

void drawOverlays() {
    ImDrawList* fg = ImGui::GetForegroundDrawList();
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    if (screen.x <= 0 || screen.y <= 0) {
        return;
    }

    // 1. Night Light Amber Warmth Filter
    if (nightLight) {
        fg->AddRectFilled(ImVec2(0, 0), screen, IM_COL32(255, 145, 30, 42));
    }

    // 2. Brightness Dimming Filter
    if (brightnessLevel < 100) {
        int alpha = static_cast<int>((100 - brightnessLevel) * 2.15f);
        if (alpha > 225) alpha = 225;
        fg->AddRectFilled(ImVec2(0, 0), screen, IM_COL32(0, 0, 0, alpha));
    }

    // 3. Volume On-Screen Display (OSD) pill (Windows 11 style)
    if (volumeLevel != lastVolume) {
        volumeOsdTimer = 1.6f;
        lastVolume = volumeLevel;
    }

    if (volumeOsdTimer > 0.0f) {
        volumeOsdTimer -= ImGui::GetIO().DeltaTime;
        float alphaMult = 1.0f;
        if (volumeOsdTimer < 0.3f) {
            alphaMult = volumeOsdTimer / 0.3f;
        }
        int bgAlpha = static_cast<int>(240 * alphaMult);
        int borderAlpha = static_cast<int>(45 * alphaMult);
        int textAlpha = static_cast<int>(255 * alphaMult);

        ImVec2 osdSize(220.0f, 44.0f);
        ImVec2 osdPos(screen.x * 0.5f - osdSize.x * 0.5f, screen.y - HEIGHT - 68.0f);
        if (config.taskbarTop) {
            osdPos.y = HEIGHT + 24.0f;
        }

        fg->AddRectFilled(osdPos, osdPos + osdSize, IM_COL32(32, 32, 35, bgAlpha), 10.0f);
        fg->AddRect(osdPos, osdPos + osdSize, IM_COL32(255, 255, 255, borderAlpha), 10.0f, 0, 1.0f);

        // Speaker icon
        ImVec2 iconPos = osdPos + ImVec2(24.0f, 22.0f);
        fg->AddRectFilled(iconPos + ImVec2(-6, -3), iconPos + ImVec2(-3, 3), IM_COL32(255, 255, 255, textAlpha));
        fg->AddQuadFilled(iconPos + ImVec2(-3, -3), iconPos + ImVec2(1, -6), iconPos + ImVec2(1, 6), iconPos + ImVec2(-3, 3), IM_COL32(255, 255, 255, textAlpha));
        if (volumeLevel == 0) {
            fg->AddLine(iconPos + ImVec2(3, -4), iconPos + ImVec2(8, 4), IM_COL32(240, 70, 70, textAlpha), 2.0f);
            fg->AddLine(iconPos + ImVec2(3, 4), iconPos + ImVec2(8, -4), IM_COL32(240, 70, 70, textAlpha), 2.0f);
        } else {
            fg->PathArcTo(iconPos + ImVec2(1, 0), 4.5f, -0.9f, 0.9f);
            fg->PathStroke(IM_COL32(255, 255, 255, textAlpha), 0, 1.5f);
            if (volumeLevel > 40) {
                fg->PathArcTo(iconPos + ImVec2(1, 0), 8.0f, -0.9f, 0.9f);
                fg->PathStroke(IM_COL32(255, 255, 255, textAlpha), 0, 1.5f);
            }
        }

        // Progress bar track
        ImVec2 barStart(osdPos.x + 42.0f, osdPos.y + 20.0f);
        float barWidth = 118.0f;
        fg->AddRectFilled(barStart, barStart + ImVec2(barWidth, 5.0f), IM_COL32(80, 80, 85, bgAlpha), 2.5f);
        if (volumeLevel > 0) {
            float fillW = barWidth * (volumeLevel / 100.0f);
            fg->AddRectFilled(barStart, barStart + ImVec2(fillW, 5.0f), IM_COL32(0, 120, 215, textAlpha), 2.5f);
        }

        // Percentage text
        char pctBuf[16];
        snprintf(pctBuf, sizeof(pctBuf), "%d%%", volumeLevel);
        fg->AddText(osdPos + ImVec2(170.0f, 14.0f), IM_COL32(240, 240, 240, textAlpha), pctBuf);
    }
}

bool isWifiEnabled() {
    return wifiEnabled;
}

int getVolume() {
    return volumeLevel;
}

int getBrightness() {
    return brightnessLevel;
}

bool isNightLight() {
    return nightLight;
}

}
