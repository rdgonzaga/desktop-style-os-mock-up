#include "Config.h"

#include <cctype>
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <fstream>

namespace {

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r");
    if (start == std::string::npos) {
        return "";
    }
    return s.substr(start, s.find_last_not_of(" \t\r") - start + 1);
}

std::string toLower(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

bool parseBool(const std::string& value, bool& out) {
    std::string v = toLower(value);
    if (v == "true" || v == "yes" || v == "on" || v == "1") {
        out = true;
        return true;
    }
    if (v == "false" || v == "no" || v == "off" || v == "0") {
        out = false;
        return true;
    }
    return false;
}

bool parseInt(const std::string& value, int min, int max, int& out) {
    int number = 0;
    const char* end = value.data() + value.size();
    auto [last, error] = std::from_chars(value.data(), end, number);
    if (error != std::errc() || last != end || number < min || number > max) {
        return false;
    }
    out = number;
    return true;
}

bool parseSeconds(const std::string& value, float& out) {
    char* end = nullptr;
    float number = std::strtof(value.c_str(), &end);
    if (value.empty() || *end != '\0' || number < 0.0f || number > 60.0f) {
        return false;
    }
    out = number;
    return true;
}

bool parseSize(const std::string& value, WindowSize& out) {
    size_t x = toLower(value).find('x');
    if (x == std::string::npos) {
        return false;
    }
    WindowSize size{};
    if (!parseInt(trim(value.substr(0, x)), 360, 7680, size.width) ||
        !parseInt(trim(value.substr(x + 1)), 220, 4320, size.height)) {
        return false;
    }
    out = size;
    return true;
}

bool parseClock(const std::string& value, bool& desktop, bool& top, bool& right) {
    std::string v = toLower(value);
    if (v == "taskbar") {
        desktop = false;
        return true;
    }
    if (v != "top_left" && v != "top_right" && v != "bottom_left" && v != "bottom_right") {
        return false;
    }
    desktop = true;
    top = v.rfind("top", 0) == 0;
    right = v.find("right") != std::string::npos;
    return true;
}

bool parseSpeed(const std::string& value, int& out) {
    const char* names[] = {"high", "normal", "low", "paused"};
    std::string v = toLower(value);
    for (int i = 0; i < 4; i++) {
        if (v == names[i]) {
            out = i;
            return true;
        }
    }
    return false;
}

bool parseText(const std::string& value, std::string& out) {
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        out = value.substr(1, value.size() - 2);
    } else {
        out = value;
    }
    return true;
}

bool parseName(const std::string& value, std::string& out) {
    std::string text;
    parseText(value, text);
    if (text.empty()) {
        return false;
    }
    out = text;
    return true;
}

bool parseTaskbar(const std::string& value, bool& top) {
    std::string v = toLower(value);
    if (v != "top" && v != "bottom") {
        return false;
    }
    top = v == "top";
    return true;
}

}

void loadConfig(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        std::fprintf(stderr, "%s not found, using the defaults\n", path.c_str());
        return;
    }

    std::string line;
    int number = 0;
    while (std::getline(file, line)) {
        number++;
        if (number == 1 && line.rfind("\xEF\xBB\xBF", 0) == 0) {
            line.erase(0, 3);
        }
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t equals = line.find('=');
        if (equals == std::string::npos) {
            std::fprintf(stderr, "%s line %d: expected key = value\n", path.c_str(), number);
            continue;
        }
        std::string key = toLower(trim(line.substr(0, equals)));
        std::string value = trim(line.substr(equals + 1));

        bool known = true;
        bool valid = false;
        if (key == "window_width") {
            valid = parseInt(value, 640, 7680, config.windowWidth);
        } else if (key == "window_height") {
            valid = parseInt(value, 480, 4320, config.windowHeight);
        } else if (key == "fullscreen") {
            valid = parseBool(value, config.fullscreen);
        } else if (key == "os_name") {
            valid = parseText(value, config.osName);
        } else if (key == "user_name") {
            valid = parseName(value, config.userName);
        } else if (key == "wallpaper") {
            valid = parseText(value, config.wallpaper);
        } else if (key == "desktop_icons") {
            valid = parseBool(value, config.desktopIcons);
        } else if (key == "taskbar_position") {
            valid = parseTaskbar(value, config.taskbarTop);
        } else if (key == "taskbar_centered") {
            valid = parseBool(value, config.taskbarCentered);
        } else if (key == "clock_24h") {
            valid = parseBool(value, config.clock24h);
        } else if (key == "clock_seconds") {
            valid = parseBool(value, config.clockSeconds);
        } else if (key == "clock_position") {
            valid = parseClock(value, config.clockOnDesktop, config.clockTop, config.clockRight);
        } else if (key == "date_format") {
            valid = parseName(value, config.dateFormat);
        } else if (key == "volume") {
            valid = parseInt(value, 0, 100, config.volume);
        } else if (key == "brightness") {
            valid = parseInt(value, 10, 100, config.brightness);
        } else if (key == "wifi") {
            valid = parseBool(value, config.wifi);
        } else if (key == "bluetooth") {
            valid = parseBool(value, config.bluetooth);
        } else if (key == "night_light") {
            valid = parseBool(value, config.nightLight);
        } else if (key == "terminal_size") {
            valid = parseSize(value, config.terminalSize);
        } else if (key == "file_explorer_size") {
            valid = parseSize(value, config.explorerSize);
        } else if (key == "task_manager_size") {
            valid = parseSize(value, config.taskManagerSize);
        } else if (key == "process_file") {
            valid = parseText(value, config.processFile);
        } else if (key == "total_memory_mb") {
            valid = parseInt(value, 256, 1048576, config.totalMemoryMb);
        } else if (key == "update_speed") {
            valid = parseSpeed(value, config.updateSpeed);
        } else if (key == "boot_screens") {
            valid = parseBool(value, config.bootScreens);
        } else if (key == "cpu_name") {
            valid = parseText(value, config.cpuName);
        } else if (key == "bios_memory_kb") {
            valid = parseInt(value, 64, 16777216, config.biosMemoryKb);
        } else if (key == "fun_fact") {
            valid = parseText(value, config.funFact);
        } else if (key == "splash_seconds") {
            valid = parseSeconds(value, config.splashSeconds);
        } else {
            known = false;
        }

        if (!known) {
            std::fprintf(stderr, "%s line %d: unknown key '%s', ignored\n", path.c_str(), number, key.c_str());
        } else if (!valid) {
            std::fprintf(stderr, "%s line %d: bad value '%s' for %s, kept the default\n", path.c_str(), number,
                         value.c_str(), key.c_str());
        }
    }
}
