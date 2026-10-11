#pragma once

#include "imgui.h"

namespace taskbar {

inline constexpr float HEIGHT = 40.0f;

void init();
ImVec2 workMin();
ImVec2 workMax();

void draw();
void drawOverlays();

bool isWifiEnabled();
int getVolume();
int getBrightness();
bool isNightLight();

}
