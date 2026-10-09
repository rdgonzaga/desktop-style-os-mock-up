#include "shell/Boot.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iterator>
#include <string>

#include "imgui.h"

#include "Config.h"
#include "core/Power.h"
#include "core/Theme.h"

namespace boot {

namespace {

enum class Stage { Bios, Splash };

const char* BIOS_HEADER[] = {
    "CSOPESY Megatrends BIOS v1.0",
    "Copyright (C) 2026, CSOPESY Megatrends, Inc.",
    "",
    "CSOPESY-PC BIOS Date: 09/18/26",
    "CPU : Intel(R) Pentium(R) III CPU 1000MHz",
    "Speed : 1000 MHz",
    "",
};

const char* BIOS_DRIVES[] = {
    "",
    "Detecting Primary Master ..... CSOPESY HDD 40GB",
    "Detecting Primary Slave ...... None",
    "Detecting Secondary Master ... CD-ROM",
    "Detecting Secondary Slave .... None",
    "",
    "Fun fact: the first computer bug was a real moth, taped into a logbook in 1947.",
};

const char* LOGO[] = {
    R"(  ____   ____     ___    ____    _____   ____   __   __)",
    R"( / ___| / ___|   / _ \  |  _ \  | ____| / ___|  \ \ / /)",
    R"(| |     \___ \  | | | | | |_) | |  _|   \___ \   \ V /)",
    R"(| |___   ___) | | |_| | |  __/  | |___   ___) |   | |)",
    R"( \____| |____/   \___/  |_|     |_____| |____/    |_|)",
};

const char* CREDITS = "Rainer Gonzaga  |  Aaron James Gonzales  |  Mariel Yasumuro  |  Richmond Jose Ramos";

const float LINE_SECONDS = 0.15f;
const float DRIVE_SECONDS = 0.35f;
const float MEMORY_SECONDS = 1.5f;
const int MEMORY_KB = 64000;
const float BIOS_HOLD_SECONDS = 1.2f;
const float SPLASH_SECONDS = 3.5f;
const float SHUTDOWN_SECONDS = 2.5f;
const float BIOS_TEXT = 20.0f;

const ImU32 BLACK = IM_COL32(0, 0, 0, 255);
const ImU32 GRAY = IM_COL32(200, 200, 200, 255);
const ImU32 WHITE = IM_COL32(255, 255, 255, 255);
const ImU32 DIM = IM_COL32(130, 130, 130, 255);
const ImU32 BLUE = IM_COL32(40, 110, 230, 255);

Stage stage = Stage::Bios;
double stageStart = -1.0;
double shutdownStart = -1.0;

float biosSeconds() {
    return std::size(BIOS_HEADER) * LINE_SECONDS + MEMORY_SECONDS + std::size(BIOS_DRIVES) * DRIVE_SECONDS +
           BIOS_HOLD_SECONDS;
}

bool skipPressed() {
    for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key++) {
        if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(key), false)) {
            return true;
        }
    }
    return false;
}

void nextStage() {
    if (stage == Stage::Bios) {
        stage = Stage::Splash;
        stageStart = ImGui::GetTime();
    } else {
        power::state = power::State::Running;
    }
}

int visibleLines(float elapsed, float perLine, int count) {
    return std::clamp(static_cast<int>(elapsed / perLine) + 1, 0, count);
}

void drawBios(ImDrawList* draw, ImVec2 screen, float elapsed) {
    ImFont* font = theme::mono();
    float line = BIOS_TEXT * 1.25f;
    ImVec2 pos(40, 36);
    auto print = [&](const char* text, ImU32 color) {
        draw->AddText(font, BIOS_TEXT, pos, color, text);
        pos.y += line;
    };

    ImVec2 badge(screen.x - 200, 36);
    draw->AddRect(badge, badge + ImVec2(160, 64), IM_COL32(60, 200, 90, 255), 0.0f, 2.0f);
    draw->AddText(font, BIOS_TEXT, badge + ImVec2(30, 10), IM_COL32(60, 200, 90, 255), "CSOPESY");
    draw->AddText(font, BIOS_TEXT * 0.7f, badge + ImVec2(30, 36), IM_COL32(60, 200, 90, 255), "Energy Ally");

    int header = visibleLines(elapsed, LINE_SECONDS, static_cast<int>(std::size(BIOS_HEADER)));
    for (int i = 0; i < header; i++) {
        print(BIOS_HEADER[i], i == 0 ? WHITE : GRAY);
    }
    float afterHeader = elapsed - std::size(BIOS_HEADER) * LINE_SECONDS;
    if (afterHeader < 0) {
        return;
    }

    int kb = std::min(MEMORY_KB, static_cast<int>(afterHeader / MEMORY_SECONDS * MEMORY_KB) / 64 * 64);
    char memory[48];
    std::snprintf(memory, sizeof(memory), "Memory Test : %dK%s", kb, kb == MEMORY_KB ? " OK" : "");
    print(memory, GRAY);
    float afterMemory = afterHeader - MEMORY_SECONDS;
    if (afterMemory < 0) {
        return;
    }

    int drives = visibleLines(afterMemory, DRIVE_SECONDS, static_cast<int>(std::size(BIOS_DRIVES)));
    for (int i = 0; i < drives; i++) {
        print(BIOS_DRIVES[i], GRAY);
    }

    draw->AddText(font, BIOS_TEXT, ImVec2(40, screen.y - 50), DIM, "Press any key or click to skip");
}

void drawSplash(ImDrawList* draw, ImVec2 screen, float elapsed) {
    ImFont* font = theme::mono();
    float widest = 0;
    for (const char* row : LOGO) {
        widest = std::max(widest, font->CalcTextSizeA(22.0f, FLT_MAX, 0, row).x);
    }
    float size = std::min(22.0f, 22.0f * screen.x * 0.85f / widest);
    float rowHeight = size * 1.1f;
    float logoWidth = widest * size / 22.0f;
    ImVec2 pos((screen.x - logoWidth) / 2, screen.y * 0.32f);
    for (const char* row : LOGO) {
        draw->AddText(font, size, pos, WHITE, row);
        pos.y += rowHeight;
    }

    int dots = static_cast<int>(elapsed / 0.4f) % 4;
    char loading[16];
    std::snprintf(loading, sizeof(loading), "Loading%.*s", dots, "...");
    ImVec2 loadingSize = font->CalcTextSizeA(18.0f, FLT_MAX, 0, "Loading...");
    draw->AddText(font, 18.0f, ImVec2((screen.x - loadingSize.x) / 2, pos.y + 30), GRAY, loading);

    ImVec2 barSize(200, 18);
    ImVec2 barMin((screen.x - barSize.x) / 2, pos.y + 70);
    draw->AddRect(barMin, barMin + barSize, DIM, 4.0f, 1.5f);
    float travel = barSize.x + 3 * 12;
    float offset = std::fmod(elapsed * 120.0f, travel) - 3 * 12;
    draw->PushClipRect(barMin + ImVec2(2, 2), barMin + barSize - ImVec2(2, 2), true);
    for (int i = 0; i < 3; i++) {
        ImVec2 block(barMin.x + offset + i * 12, barMin.y + 4);
        draw->AddRectFilled(block, block + ImVec2(9, barSize.y - 8), BLUE, 2);
    }
    draw->PopClipRect();

    ImVec2 creditsSize = ImGui::CalcTextSize(CREDITS);
    draw->AddText(ImVec2((screen.x - creditsSize.x) / 2, screen.y - 60), DIM, CREDITS);
}

}

void draw() {
    double now = ImGui::GetTime();
    if (stageStart < 0) {
        stageStart = now;
    }
    float elapsed = static_cast<float>(now - stageStart);
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    draw->AddRectFilled(ImVec2(0, 0), screen, BLACK);

    if (stage == Stage::Bios) {
        drawBios(draw, screen, elapsed);
    } else {
        drawSplash(draw, screen, elapsed);
    }
    float duration = stage == Stage::Bios ? biosSeconds() : SPLASH_SECONDS;
    if (elapsed >= duration || skipPressed()) {
        nextStage();
    }
}

void drawShutdown() {
    double now = ImGui::GetTime();
    if (shutdownStart < 0) {
        shutdownStart = now;
    }
    float elapsed = static_cast<float>(now - shutdownStart);
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    ImDrawList* draw = ImGui::GetBackgroundDrawList();

    float band = screen.y * 0.14f;
    ImU32 light = IM_COL32(90, 126, 220, 255);
    ImU32 dark = IM_COL32(0, 48, 156, 255);
    draw->AddRectFilledMultiColor(ImVec2(0, 0), screen, light, light, dark, dark);
    draw->AddRectFilled(ImVec2(0, 0), ImVec2(screen.x, band), dark);
    draw->AddRectFilled(ImVec2(0, screen.y - band), screen, dark);
    draw->AddLine(ImVec2(0, band), ImVec2(screen.x, band), IM_COL32(150, 180, 240, 255), 2.0f);
    draw->AddLine(ImVec2(0, screen.y - band), ImVec2(screen.x, screen.y - band), IM_COL32(230, 140, 50, 255), 2.0f);

    ImFont* font = ImGui::GetFont();
    const char* name = config.osName.c_str();
    ImVec2 nameSize = font->CalcTextSizeA(44.0f, FLT_MAX, 0, name);
    ImVec2 namePos((screen.x - nameSize.x) / 2, screen.y / 2 - nameSize.y);
    draw->AddText(font, 44.0f, namePos, WHITE, name);

    int dots = static_cast<int>(elapsed / 0.4f) % 4;
    const char* message = elapsed < SHUTDOWN_SECONDS / 2 ? "Saving your settings" : "Shutting down";
    char status[48];
    std::snprintf(status, sizeof(status), "%s%.*s", message, dots, "...");
    std::string full = std::string(message) + "...";
    ImVec2 statusSize = font->CalcTextSizeA(22.0f, FLT_MAX, 0, full.c_str());
    draw->AddText(font, 22.0f, ImVec2((screen.x - statusSize.x) / 2, screen.y / 2 + 16), WHITE, status);

    if (elapsed >= SHUTDOWN_SECONDS) {
        power::state = power::State::Off;
    }
}

}
