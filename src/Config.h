#pragma once

#include <string>

struct WindowSize {
    int width;
    int height;
};

struct Config {
    int windowWidth = 1280;
    int windowHeight = 720;
    bool fullscreen = false;

    std::string osName = "CSOPESY OS v1.0";
    std::string userName = "csopesy";
    std::string wallpaper = "assets/wallpaper.jpg";
    bool desktopIcons = true;
    bool taskbarTop = false;
    bool taskbarCentered = true;

    bool clock24h = false;
    bool clockSeconds = true;
    bool clockOnDesktop = false;
    bool clockTop = true;
    bool clockRight = true;
    std::string dateFormat = "%A, %B %d, %Y";

    int volume = 75;
    int brightness = 85;
    bool wifi = true;
    bool bluetooth = true;
    bool nightLight = false;

    WindowSize terminalSize{640, 400};
    WindowSize explorerSize{820, 480};
    WindowSize taskManagerSize{780, 500};
    WindowSize settingsSize{560, 480};
    WindowSize notepadSize{600, 420};

    std::string processFile = "processes.txt";
    int totalMemoryMb = 8192;
    int updateSpeed = 1;

    bool bootScreens = true;
    std::string cpuName = "Intel(R) Pentium(R) III CPU 1000MHz";
    int biosMemoryKb = 64000;
    std::string funFact = "the first computer bug was a real moth, taped into a logbook in 1947.";
    float splashSeconds = 3.5f;
};

inline Config config;

void loadConfig(const std::string& path);
