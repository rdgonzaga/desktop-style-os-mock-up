#include "apps/Terminal.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <string>
#include <vector>

#include "imgui.h"

#include "Config.h"
#include "apps/TaskManager.h"
#include "core/Apps.h"
#include "core/Clock.h"
#include "core/Theme.h"
#include "shell/Taskbar.h"

namespace terminal {

namespace {

struct Command {
    const char* name;
    const char* usage;
    const char* description;
    void (*run)(const std::string& args);
};

const char* PROMPT = "C:\\Users\\csopesy>";
const ImU32 SCREEN_BG = IM_COL32(12, 12, 12, 255);
const ImU32 SCREEN_TEXT = IM_COL32(204, 204, 204, 255);

std::vector<std::string> lines;
std::vector<std::string> history;
int historyPos = -1;
char input[256] = "";
bool started = false;
bool scrollToBottom = false;

const std::vector<Command>& commands();

void print(const std::string& line) {
    lines.push_back(line);
    scrollToBottom = true;
}

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t");
    if (start == std::string::npos) {
        return "";
    }
    return s.substr(start, s.find_last_not_of(" \t") - start + 1);
}

std::string toLower(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

std::string appNames() {
    std::string names;
    for (const App& app : apps::all()) {
        names += names.empty() ? app.name : std::string(", ") + app.name;
    }
    return names;
}

void cmdHelp(const std::string&) {
    for (const Command& command : commands()) {
        char line[160];
        std::snprintf(line, sizeof(line), "%-14s %s", command.usage, command.description);
        print(line);
    }
}

void cmdClear(const std::string&) {
    lines.clear();
}

void cmdEcho(const std::string& args) {
    print(args);
}

void cmdDate(const std::string&) {
    print(sysclock::dateTime());
}

void cmdVer(const std::string&) {
    print(config.osName);
}

void cmdWhoami(const std::string&) {
    print("csopesy-pc\\csopesy");
}

void cmdPs(const std::string&) {
    std::vector<taskmanager::Process> list = taskmanager::processes();
    print("Name                                    CPU      Memory");
    print("------------------------------------ ------ -----------");
    for (const taskmanager::Process& process : list) {
        char line[160];
        std::snprintf(line, sizeof(line), "%-36.36s %5.1f%% %8.1f MB", process.name.c_str(), process.cpu,
                      process.memory);
        print(line);
    }
    print(std::to_string(list.size()) + " processes");
}

void cmdOpen(const std::string& args) {
    App* app = apps::find(args);
    if (!app) {
        print(args.empty() ? "Usage: open <app>" : "'" + args + "' is not an app.");
        print("Apps: " + appNames());
        return;
    }
    apps::open(*app);
    print("Opening " + std::string(app->name) + "...");
}

void cmdPing(const std::string& args) {
    if (!taskbar::isWifiEnabled()) {
        print("Ping request could not find host. Error: Network is unreachable (Wi-Fi is turned off).");
        return;
    }
    std::string target = args.empty() ? "8.8.8.8" : args;
    print("Pinging " + target + " with 32 bytes of data:");
    print("Reply from " + target + ": bytes=32 time=14ms TTL=117");
    print("Reply from " + target + ": bytes=32 time=12ms TTL=117");
    print("Reply from " + target + ": bytes=32 time=15ms TTL=117");
    print("Ping statistics for " + target + ": Packets: Sent = 3, Received = 3, Lost = 0 (0% loss)");
}

void cmdIpconfig(const std::string&) {
    print("CSOPESY IP Configuration:");
    print("");
    print("Wireless LAN adapter Wi-Fi:");
    if (!taskbar::isWifiEnabled()) {
        print("   Media State . . . . . . . . . . . : Media disconnected");
        print("   Connection-specific DNS Suffix  . : ");
    } else {
        print("   Connection-specific DNS Suffix  . : localdomain");
        print("   IPv4 Address. . . . . . . . . . . : 192.168.1.105");
        print("   Subnet Mask . . . . . . . . . . . : 255.255.255.0");
        print("   Default Gateway . . . . . . . . . : 192.168.1.1");
    }
}

void cmdExit(const std::string&) {
    if (App* self = apps::find("Terminal")) {
        apps::close(*self);
    }
    started = false;
}

const std::vector<Command>& commands() {
    static const std::vector<Command> table = {
        {"help",     "help",        "lists the commands",                    cmdHelp},
        {"clear",    "clear",       "clears the screen",                     cmdClear},
        {"echo",     "echo <text>", "prints the text back",                 cmdEcho},
        {"date",     "date",        "shows the current date and time",       cmdDate},
        {"ver",      "ver",         "shows the os version",                  cmdVer},
        {"whoami",   "whoami",      "shows the current user",                cmdWhoami},
        {"ps",       "ps",          "lists the running processes",           cmdPs},
        {"open",     "open <app>",  "opens an app, e.g. open task manager",  cmdOpen},
        {"ping",     "ping <host>", "tests network connectivity",            cmdPing},
        {"ipconfig", "ipconfig",    "displays network IP configuration",     cmdIpconfig},
        {"exit",     "exit",        "closes the terminal",                   cmdExit},
    };
    return table;
}

void execute(const std::string& raw) {
    std::string line = trim(raw);
    print(PROMPT + line);
    if (line.empty()) {
        return;
    }
    history.push_back(line);
    historyPos = -1;

    size_t space = line.find(' ');
    std::string name = toLower(line.substr(0, space));
    std::string args = space == std::string::npos ? "" : trim(line.substr(space + 1));
    bool found = false;
    for (const Command& command : commands()) {
        if (name == command.name) {
            command.run(args);
            found = true;
            break;
        }
    }
    if (!found) {
        print("'" + name + "' is not recognized as an internal or external command,");
        print("operable program or batch file.");
    }
    if (!lines.empty()) {
        print("");
    }
}

int onHistory(ImGuiInputTextCallbackData* data) {
    if (history.empty()) {
        return 0;
    }
    int last = static_cast<int>(history.size()) - 1;
    if (data->EventKey == ImGuiKey_UpArrow) {
        historyPos = historyPos < 0 ? last : std::max(0, historyPos - 1);
    } else if (data->EventKey == ImGuiKey_DownArrow) {
        if (historyPos < 0) {
            return 0;
        }
        historyPos = historyPos >= last ? -1 : historyPos + 1;
    }
    data->DeleteChars(0, data->BufTextLen);
    data->InsertChars(0, historyPos < 0 ? "" : history[historyPos].c_str());
    return 0;
}

void greet() {
    lines.clear();
    input[0] = '\0';
    print(config.osName);
    print("(c) 2026 CSOPESY Group. All rights reserved.");
    print("Type 'help' to see the commands.");
    print("");
    started = true;
}

}

void draw() {
    if (!started) {
        greet();
    }
    bool windowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

    ImGui::PushFont(theme::mono(), 0.0f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, SCREEN_BG);
    ImGui::PushStyleColor(ImGuiCol_Text, SCREEN_TEXT);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, SCREEN_BG);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, SCREEN_BG);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, SCREEN_BG);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::BeginChild("screen", ImVec2(0, 0), ImGuiChildFlags_AlwaysUseWindowPadding);

    for (const std::string& line : lines) {
        ImGui::TextUnformatted(line.c_str());
    }
    ImGui::TextUnformatted(PROMPT);
    ImGui::SameLine(0.0f, 0.0f);
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (windowFocused && !ImGui::IsAnyItemActive() && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        ImGui::SetKeyboardFocusHere();
    }
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory;
    if (ImGui::InputText("##input", input, sizeof(input), flags, onHistory)) {
        std::string line = input;
        input[0] = '\0';
        execute(line);
    }
    if (scrollToBottom) {
        ImGui::SetScrollHereY(1.0f);
        scrollToBottom = false;
    }

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(5);
    ImGui::PopFont();
}

}
