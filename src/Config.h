#pragma once

#include <string>

struct Config {
    int windowWidth = 1280;
    int windowHeight = 720;

    std::string osName = "CSOPESY OS v1.0";
    std::string wallpaper = "assets/wallpaper.jpg";

    bool clock24h = false;
    bool clockSeconds = true;
};

inline Config config;
