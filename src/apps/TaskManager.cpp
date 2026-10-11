#include "apps/TaskManager.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <random>
#include <sstream>

#include "imgui.h"

#include "Config.h"
#include "core/Apps.h"

namespace taskmanager {

namespace {

enum Column { NAME, STATUS, CPU, MEMORY, DISK, NETWORK };

struct Tracked {
    Process base;
    Process now;
};

struct Totals {
    float cpu = 0.0f;
    float memory = 0.0f;
    float disk = 0.0f;
    float network = 0.0f;
};

const float DISK_LIMIT_MBS = 100.0f;
const float NETWORK_LIMIT_MBPS = 100.0f;
const int HISTORY = 60;

const float APP_USAGE[][4] = {
    {1.6f, 32.5f, 0.1f, 0.0f},
    {0.8f, 41.2f, 0.3f, 0.0f},
    {2.4f, 27.8f, 0.0f, 0.1f},
};

const char* SPEED_NAMES[] = {"High", "Normal", "Low", "Paused"};
const float SPEED_SECONDS[] = {0.5f, 1.0f, 4.0f, 0.0f};
const char* PLACEHOLDER_TABS[] = {"App history", "Startup", "Users", "Details", "Services"};

const ImU32 GROUP_TEXT = IM_COL32(96, 175, 255, 255);
const ImU32 GRAPH_LINE = IM_COL32(0, 150, 240, 255);

std::mt19937 rng{std::random_device{}()};

Tracked background(const char* name, float cpu, float memory, float disk, float network, bool suspended = false) {
    Process process{name, Icon::Window, suspended, cpu, memory, disk, network};
    return {process, process};
}

std::vector<Tracked> backgroundRows = {
    background("Antimalware Service Executable", 0.8f, 142.3f, 0.1f, 0.0f),
    background("Client Server Runtime Process", 0.2f, 1.1f, 0.0f, 0.0f),
    background("COM Surrogate", 0.1f, 2.6f, 0.0f, 0.0f),
    background("Desktop Window Manager", 1.9f, 58.4f, 0.0f, 0.0f),
    background("Runtime Broker", 0.1f, 4.8f, 0.0f, 0.0f),
    background("Search", 0.0f, 38.0f, 0.0f, 0.0f, true),
    background("Service Host: Local System", 0.4f, 24.7f, 0.2f, 0.0f),
    background("Service Host: Network Service", 0.1f, 6.2f, 0.0f, 0.3f),
    background("Spooler SubSystem App", 0.1f, 3.1f, 0.0f, 0.0f),
    background("System", 0.6f, 0.1f, 0.4f, 0.1f),
    background("System interrupts", 0.3f, 0.0f, 0.0f, 0.0f),
    background("Windows Audio Device Graph Isolation", 0.1f, 7.5f, 0.0f, 0.0f),
    background("Windows Explorer", 0.5f, 46.9f, 0.1f, 0.0f),
};

std::vector<Tracked> appRows;
std::vector<float> cpuHistory(HISTORY, 0.0f);
std::vector<float> memoryHistory(HISTORY, 0.0f);
double lastTick = -1.0;
int speed = 1;
std::string selected;

float totalMemoryMb() {
    return static_cast<float>(config.totalMemoryMb);
}

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r");
    if (start == std::string::npos) {
        return "";
    }
    return s.substr(start, s.find_last_not_of(" \t\r") - start + 1);
}

bool parseUsage(const std::string& text, float& out) {
    char* end = nullptr;
    float number = std::strtof(text.c_str(), &end);
    if (text.empty() || *end != '\0' || number < 0.0f || number > 1000000.0f) {
        return false;
    }
    out = number;
    return true;
}

bool parseProcess(const std::string& line, Tracked& out) {
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;
    while (std::getline(stream, field, ',')) {
        fields.push_back(trim(field));
    }
    if (fields.size() < 5 || fields.size() > 6 || fields[0].empty()) {
        return false;
    }
    float usage[4];
    for (int i = 0; i < 4; i++) {
        if (!parseUsage(fields[i + 1], usage[i])) {
            return false;
        }
    }
    bool suspended = false;
    if (fields.size() == 6) {
        std::string status = fields[5];
        for (char& c : status) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        if (status != "suspended" && status != "running") {
            return false;
        }
        suspended = status == "suspended";
    }
    if (suspended) {
        usage[0] = usage[2] = usage[3] = 0.0f;
    }
    out = background(fields[0].c_str(), std::min(usage[0], 100.0f), usage[1], usage[2], usage[3], suspended);
    return true;
}

float randomBetween(float low, float high) {
    return std::uniform_real_distribution<float>(low, high)(rng);
}

void drift(Tracked& row) {
    const Process& base = row.base;
    Process& now = row.now;
    now.memory = base.memory * randomBetween(0.97f, 1.03f);
    if (base.suspended) {
        return;
    }
    now.cpu = base.cpu * randomBetween(0.3f, 1.8f);
    now.disk = randomBetween(0.0f, 1.0f) < 0.3f ? base.disk * randomBetween(0.5f, 3.0f) : 0.0f;
    now.network = randomBetween(0.0f, 1.0f) < 0.3f ? base.network * randomBetween(0.5f, 3.0f) : 0.0f;
}

void syncApps() {
    std::vector<App>& table = apps::all();
    if (appRows.size() == table.size()) {
        return;
    }
    appRows.clear();
    for (size_t i = 0; i < table.size(); i++) {
        const float* usage = APP_USAGE[i % std::size(APP_USAGE)];
        Process process{table[i].name, table[i].icon, false, usage[0], usage[1], usage[2], usage[3]};
        appRows.push_back({process, process});
    }
}

std::vector<Process> openApps() {
    syncApps();
    std::vector<Process> list;
    std::vector<App>& table = apps::all();
    for (size_t i = 0; i < table.size(); i++) {
        if (table[i].open) {
            list.push_back(appRows[i].now);
        }
    }
    return list;
}

Totals percentTotals(const std::vector<Process>& list) {
    Totals sum;
    for (const Process& process : list) {
        sum.cpu += process.cpu;
        sum.memory += process.memory;
        sum.disk += process.disk;
        sum.network += process.network;
    }
    sum.cpu = std::min(sum.cpu, 100.0f);
    sum.memory = std::min(sum.memory / totalMemoryMb() * 100.0f, 100.0f);
    sum.disk = std::min(sum.disk / DISK_LIMIT_MBS * 100.0f, 100.0f);
    sum.network = std::min(sum.network / NETWORK_LIMIT_MBPS * 100.0f, 100.0f);
    return sum;
}

void record(std::vector<float>& history, float value) {
    history.erase(history.begin());
    history.push_back(value);
}

void tick() {
    syncApps();
    for (Tracked& row : appRows) {
        drift(row);
    }
    for (Tracked& row : backgroundRows) {
        drift(row);
    }
    Totals total = percentTotals(processes());
    record(cpuHistory, total.cpu);
    record(memoryHistory, total.memory);
    lastTick = ImGui::GetTime();
}

void update() {
    float every = SPEED_SECONDS[speed];
    if (lastTick < 0.0 || (every > 0.0f && ImGui::GetTime() - lastTick >= every)) {
        tick();
    }
}

void endTask(const std::string& name) {
    for (App& app : apps::all()) {
        if (name == app.name) {
            apps::close(app);
            return;
        }
    }
    backgroundRows.erase(std::remove_if(backgroundRows.begin(), backgroundRows.end(),
                                        [&](const Tracked& row) { return row.now.name == name; }),
                         backgroundRows.end());
}

int compare(const Process& a, const Process& b, ImGuiID column) {
    auto order = [](float x, float y) { return (x > y) - (x < y); };
    switch (column) {
    case STATUS:
        return static_cast<int>(a.suspended) - static_cast<int>(b.suspended);
    case CPU:
        return order(a.cpu, b.cpu);
    case MEMORY:
        return order(a.memory, b.memory);
    case DISK:
        return order(a.disk, b.disk);
    case NETWORK:
        return order(a.network, b.network);
    default:
        return a.name.compare(b.name);
    }
}

void sortRows(std::vector<Process>& rows, const ImGuiTableSortSpecs* specs) {
    if (!specs || specs->SpecsCount == 0) {
        return;
    }
    const ImGuiTableColumnSortSpecs& spec = specs->Specs[0];
    std::stable_sort(rows.begin(), rows.end(), [&](const Process& a, const Process& b) {
        int result = compare(a, b, spec.ColumnUserID);
        return spec.SortDirection == ImGuiSortDirection_Ascending ? result < 0 : result > 0;
    });
}

std::string header(const char* name, float percent, const char* id) {
    char text[48];
    std::snprintf(text, sizeof(text), "%.0f%%\n%s###%s", percent, name, id);
    return text;
}

ImU32 shade(float load) {
    float t = std::clamp(load, 0.0f, 1.0f);
    if (t <= 0.01f) return IM_COL32(0, 0, 0, 0);
    return IM_COL32(0, 120, 215, static_cast<int>(35 + 175 * t));
}

void rightAligned(const char* text) {
    float offset = ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(text).x;
    if (offset > 0.0f) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    }
    ImGui::TextUnformatted(text);
}

void usageCell(const char* format, float value, float full) {
    ImGui::TableNextColumn();
    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, shade(value / full));
    char text[32];
    std::snprintf(text, sizeof(text), format, value);
    rightAligned(text);
}

void groupRow(const char* title, size_t count) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::PushStyleColor(ImGuiCol_Text, GROUP_TEXT);
    ImGui::Text("%s (%d)", title, static_cast<int>(count));
    ImGui::PopStyleColor();
}

void processRow(const Process& process) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::PushID(process.name.c_str());
    if (ImGui::Selectable("##row", selected == process.name, ImGuiSelectableFlags_SpanAllColumns)) {
        selected = process.name;
    }
    ImGui::PopID();
    float line = ImGui::GetTextLineHeight();
    drawIcon(ImGui::GetWindowDrawList(), process.icon, ImVec2(pos.x + 20, pos.y + line / 2), 18.0f);
    ImGui::SetCursorScreenPos(ImVec2(pos.x + 36, pos.y));
    ImGui::TextUnformatted(process.name.c_str());

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(process.suspended ? "Suspended" : "");
    usageCell("%.1f%%", process.cpu, 20.0f);
    usageCell("%.1f MB", process.memory, 400.0f);
    usageCell("%.1f MB/s", process.disk, 5.0f);
    usageCell("%.1f Mbps", process.network, 5.0f);
}

void menuBar() {
    if (!ImGui::BeginMenuBar()) {
        return;
    }
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Exit")) {
            if (App* self = apps::find("Task Manager")) {
                apps::close(*self);
            }
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
        if (ImGui::MenuItem("Refresh now")) {
            tick();
        }
        if (ImGui::BeginMenu("Update speed")) {
            for (int i = 0; i < static_cast<int>(std::size(SPEED_NAMES)); i++) {
                if (ImGui::MenuItem(SPEED_NAMES[i], nullptr, speed == i)) {
                    speed = i;
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
}

void processesTab() {
    std::vector<Process> appList = openApps();
    std::vector<Process> backgroundList;
    for (const Tracked& row : backgroundRows) {
        backgroundList.push_back(row.now);
    }
    Totals total = percentTotals(processes());

    auto listed = [&](const Process& process) { return process.name == selected; };
    if (std::none_of(appList.begin(), appList.end(), listed) &&
        std::none_of(backgroundList.begin(), backgroundList.end(), listed)) {
        selected.clear();
    }

    ImGuiTableFlags flags = ImGuiTableFlags_Sortable | ImGuiTableFlags_SortTristate | ImGuiTableFlags_ScrollY |
                            ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuter;
    if (ImGui::BeginTable("processes", 6, flags, ImVec2(0, -ImGui::GetFrameHeightWithSpacing()))) {
        ImGuiTableColumnFlags usage = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending;
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("\nName", ImGuiTableColumnFlags_WidthStretch, 0.0f, NAME);
        ImGui::TableSetupColumn("\nStatus", ImGuiTableColumnFlags_WidthFixed, 80.0f, STATUS);
        ImGui::TableSetupColumn(header("CPU", total.cpu, "cpu").c_str(), usage, 70.0f, CPU);
        ImGui::TableSetupColumn(header("Memory", total.memory, "memory").c_str(), usage, 90.0f, MEMORY);
        ImGui::TableSetupColumn(header("Disk", total.disk, "disk").c_str(), usage, 80.0f, DISK);
        ImGui::TableSetupColumn(header("Network", total.network, "network").c_str(), usage, 80.0f, NETWORK);
        ImGui::TableHeadersRow();

        const ImGuiTableSortSpecs* specs = ImGui::TableGetSortSpecs();
        sortRows(appList, specs);
        sortRows(backgroundList, specs);

        groupRow("Apps", appList.size());
        for (const Process& process : appList) {
            processRow(process);
        }
        groupRow("Background processes", backgroundList.size());
        for (const Process& process : backgroundList) {
            processRow(process);
        }
        ImGui::EndTable();
    }

    float buttonWidth = 100.0f;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - buttonWidth);
    ImGui::BeginDisabled(selected.empty());
    if (ImGui::Button("End task", ImVec2(buttonWidth, 0))) {
        endTask(selected);
        selected.clear();
    }
    ImGui::EndDisabled();
}

void graph(const char* title, const char* subtitle, const std::vector<float>& history, const char* detail) {
    ImGui::TextUnformatted(title);
    ImGui::SameLine();
    ImGui::TextDisabled("%s", subtitle);
    ImGui::PushID(title);
    ImGui::PlotLines("##graph", history.data(), static_cast<int>(history.size()), 0, detail, 0.0f, 100.0f,
                     ImVec2(-1.0f, 110.0f));
    ImGui::PopID();
}

void performanceTab() {
    Totals total = percentTotals(processes());
    float totalGb = totalMemoryMb() / 1024.0f;
    float usedGb = total.memory / 100.0f * totalGb;
    char cpuText[48];
    char memoryText[64];
    char memorySize[32];
    std::snprintf(cpuText, sizeof(cpuText), "Utilization %.0f%%", total.cpu);
    std::snprintf(memoryText, sizeof(memoryText), "In use %.1f GB of %.1f GB (%.0f%%)", usedGb, totalGb,
                  total.memory);
    std::snprintf(memorySize, sizeof(memorySize), "%.1f GB", totalGb);

    ImGui::PushStyleColor(ImGuiCol_PlotLines, GRAPH_LINE);
    graph("CPU", config.cpuName.c_str(), cpuHistory, cpuText);
    ImGui::Spacing();
    graph("Memory", memorySize, memoryHistory, memoryText);
    ImGui::PopStyleColor();

    ImGui::Spacing();
    int seconds = static_cast<int>(ImGui::GetTime());
    ImGui::Text("Processes    %d", static_cast<int>(processes().size()));
    ImGui::Text("Up time      %d:%02d:%02d:%02d", seconds / 86400, seconds / 3600 % 24, seconds / 60 % 60,
                seconds % 60);
}

}

void init() {
    speed = config.updateSpeed;

    std::ifstream file(config.processFile);
    if (!file) {
        std::fprintf(stderr, "%s not found, using the built-in processes\n", config.processFile.c_str());
        return;
    }
    backgroundRows.clear();
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
        Tracked row;
        if (!parseProcess(line, row)) {
            std::fprintf(stderr, "%s line %d: expected name, cpu, memory, disk, network[, suspended], ignored\n",
                         config.processFile.c_str(), number);
            continue;
        }
        bool taken = std::any_of(backgroundRows.begin(), backgroundRows.end(),
                                 [&](const Tracked& other) { return other.base.name == row.base.name; });
        if (taken) {
            std::fprintf(stderr, "%s line %d: '%s' is already listed, ignored\n", config.processFile.c_str(), number,
                         row.base.name.c_str());
            continue;
        }
        backgroundRows.push_back(row);
    }
}

std::vector<Process> processes() {
    std::vector<Process> list = openApps();
    for (const Tracked& row : backgroundRows) {
        list.push_back(row.now);
    }
    return list;
}

void draw() {
    update();
    menuBar();
    if (!ImGui::BeginTabBar("tabs")) {
        return;
    }
    if (ImGui::BeginTabItem("Processes")) {
        processesTab();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Performance")) {
        performanceTab();
        ImGui::EndTabItem();
    }
    for (const char* tab : PLACEHOLDER_TABS) {
        if (ImGui::BeginTabItem(tab)) {
            ImGui::TextDisabled("Nothing to show here yet.");
            ImGui::EndTabItem();
        }
    }
    ImGui::EndTabBar();
}

}
