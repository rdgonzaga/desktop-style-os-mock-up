#pragma once

#include <string>

// Everything the quiz might ask us to change lives here, so it can later be
// read from config.txt instead of needing a recompile.
struct Config {
    int windowWidth = 1280;
    int windowHeight = 720;

    std::string osName = "CSOPESY OS v1.0";
    std::string wallpaper = "assets/wallpaper.jpg";

    bool clock24h = false;
    bool clockSeconds = true;
};

inline Config config;
