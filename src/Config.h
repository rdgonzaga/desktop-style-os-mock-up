#pragma once

#include <string>

struct Config {
    int windowWidth = 1280;
    int windowHeight = 720;
    bool fullscreen = false;

    std::string osName = "CSOPESY OS v1.0";
    std::string wallpaper = "assets/wallpaper.jpg";
    bool taskbarTop = false;
    bool taskbarCentered = true;

    bool clock24h = false;
    bool clockSeconds = true;

    bool bootScreens = true;
};

inline Config config;

void loadConfig(const std::string& path);
