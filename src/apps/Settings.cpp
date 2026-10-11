#include "apps/Settings.h"

#include <cstdio>
#include <string>

#include "imgui.h"

#include "Config.h"

namespace settings {

namespace {

std::string yesNo(bool value) {
    return value ? "true" : "false";
}

std::string size(const WindowSize& s) {
    return std::to_string(s.width) + "x" + std::to_string(s.height);
}

std::string clockPosition() {
    if (!config.clockOnDesktop) {
        return "taskbar";
    }
    return std::string(config.clockTop ? "top" : "bottom") + (config.clockRight ? "_right" : "_left");
}

void section(const char* title) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Spacing();
    ImGui::TextDisabled("%s", title);
}

void row(const char* key, const std::string& value) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(key);
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(value.c_str());
}

}

void draw() {
    ImGui::TextWrapped("These come from config.txt when the OS starts. Edit the file, save it, then press Run again.");
    ImGui::Spacing();

    ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter;
    if (!ImGui::BeginTable("settings", 2, flags)) {
        return;
    }
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed, 170.0f);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();

    section("Display");
    row("window_width", std::to_string(config.windowWidth));
    row("window_height", std::to_string(config.windowHeight));
    row("fullscreen", yesNo(config.fullscreen));
    row("os_name", config.osName);
    row("user_name", config.userName);
    row("wallpaper", config.wallpaper);
    row("desktop_icons", yesNo(config.desktopIcons));

    section("Taskbar and clock");
    row("taskbar_position", config.taskbarTop ? "top" : "bottom");
    row("taskbar_centered", yesNo(config.taskbarCentered));
    row("clock_position", clockPosition());
    row("clock_24h", yesNo(config.clock24h));
    row("clock_seconds", yesNo(config.clockSeconds));
    row("date_format", config.dateFormat);

    section("Quick settings at startup");
    row("volume", std::to_string(config.volume));
    row("brightness", std::to_string(config.brightness));
    row("wifi", yesNo(config.wifi));
    row("bluetooth", yesNo(config.bluetooth));
    row("night_light", yesNo(config.nightLight));

    section("Windows");
    row("terminal_size", size(config.terminalSize));
    row("file_explorer_size", size(config.explorerSize));
    row("task_manager_size", size(config.taskManagerSize));
    row("settings_size", size(config.settingsSize));
    row("notepad_size", size(config.notepadSize));

    section("Task Manager");
    const char* speeds[] = {"high", "normal", "low", "paused"};
    row("process_file", config.processFile);
    row("total_memory_mb", std::to_string(config.totalMemoryMb));
    row("update_speed", speeds[config.updateSpeed]);

    section("Boot");
    char seconds[16];
    std::snprintf(seconds, sizeof(seconds), "%.1f", config.splashSeconds);
    row("boot_screens", yesNo(config.bootScreens));
    row("cpu_name", config.cpuName);
    row("bios_memory_kb", std::to_string(config.biosMemoryKb));
    row("fun_fact", config.funFact);
    row("splash_seconds", seconds);

    ImGui::EndTable();
}

}
