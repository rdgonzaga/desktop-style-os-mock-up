#include "apps/FileExplorer.h"

#include <string>
#include <utility>
#include <vector>

#include "imgui.h"

#include "core/Icons.h"

namespace explorer {

namespace {

struct Entry {
    std::string name;
    Icon icon;
    std::string type;
    std::string modified;
    std::string size;
    std::vector<Entry> children;
};

using Path = std::vector<const Entry*>;

Entry folder(const char* name, const char* modified, std::vector<Entry> children = {}) {
    return {name, Icon::Folder, "File folder", modified, "", std::move(children)};
}

Entry file(const char* name, const char* type, const char* modified, const char* size) {
    return {name, Icon::File, type, modified, size, {}};
}

const Entry root = {"This PC", Icon::Computer, "", "", "", {
    folder("Desktop", "10/09/2026 11:42 PM", {
        file("CSOPESY MO4 Report.pptx", "Microsoft PowerPoint Presentation", "10/09/2026 11:40 PM", "2,416 KB"),
        file("notes.txt", "Text Document", "10/09/2026 10:15 PM", "2 KB"),
    }),
    folder("Documents", "10/08/2026 08:03 PM", {
        folder("CSOPESY", "10/09/2026 11:36 PM", {
            folder("MO3 - Marquee Console", "09/26/2026 04:12 PM", {
                file("main.cpp", "C++ Source File", "09/26/2026 04:10 PM", "3 KB"),
                file("README.txt", "Text Document", "09/26/2026 04:12 PM", "1 KB"),
            }),
            folder("MO4 - Desktop OS", "10/09/2026 11:36 PM", {
                file("CMakeLists.txt", "Text Document", "10/09/2026 11:30 PM", "2 KB"),
                file("config.txt", "Text Document", "10/09/2026 11:31 PM", "1 KB"),
                file("main.cpp", "C++ Source File", "10/09/2026 11:36 PM", "3 KB"),
            }),
        }),
        file("Budget.xlsx", "Microsoft Excel Worksheet", "09/30/2026 09:20 PM", "11 KB"),
        file("Resume.docx", "Microsoft Word Document", "08/14/2026 02:47 PM", "18 KB"),
    }),
    folder("Downloads", "10/09/2026 07:55 PM", {
        file("glfw-3.5.1.zip", "Compressed (zipped) Folder", "10/09/2026 07:52 PM", "1,320 KB"),
        file("imgui-1.92.9b.zip", "Compressed (zipped) Folder", "10/09/2026 07:53 PM", "1,840 KB"),
        file("VisualStudioSetup.exe", "Application", "10/09/2026 07:55 PM", "4,302 KB"),
    }),
    folder("Pictures", "10/09/2026 11:37 PM", {
        folder("Screenshots", "10/09/2026 11:37 PM", {
            file("Screenshot 2026-10-09 233641.png", "PNG File", "10/09/2026 11:36 PM", "312 KB"),
            file("Screenshot 2026-10-09 233644.png", "PNG File", "10/09/2026 11:36 PM", "298 KB"),
        }),
        file("bliss.jpg", "JPG File", "10/09/2026 11:50 PM", "262 KB"),
    }),
    {"Local Disk (C:)", Icon::Drive, "Local Disk", "", "476 GB", {
        folder("Program Files", "10/09/2026 11:58 PM", {
            folder("CSOPESY OS", "10/09/2026 11:58 PM", {
                file("config.txt", "Text Document", "10/09/2026 11:58 PM", "1 KB"),
                file("CsopesyOS.exe", "Application", "10/09/2026 11:58 PM", "2,184 KB"),
            }),
        }),
        folder("Users", "10/01/2026 09:00 AM", {
            folder("csopesy", "10/09/2026 11:42 PM"),
        }),
        folder("Windows", "10/01/2026 09:00 AM", {
            folder("System32", "10/01/2026 09:00 AM", {
                file("kernel32.dll", "Application extension", "10/01/2026 09:00 AM", "812 KB"),
                file("notepad.exe", "Application", "10/01/2026 09:00 AM", "352 KB"),
            }),
        }),
    }},
}};

const char* QUICK_ACCESS[] = {"Desktop", "Downloads", "Documents", "Pictures"};
const float SIDEBAR_WIDTH = 190.0f;
const float LINK_INDENT = 10.0f;

Path path = {&root};
std::vector<Path> backStack;
std::vector<Path> forwardStack;
int selected = -1;

void go(Path next) {
    backStack.push_back(path);
    forwardStack.clear();
    path = std::move(next);
    selected = -1;
}

void step(std::vector<Path>& from, std::vector<Path>& to) {
    to.push_back(path);
    path = from.back();
    from.pop_back();
    selected = -1;
}

const Entry* rootChild(const char* name) {
    for (const Entry& child : root.children) {
        if (child.name == name) {
            return &child;
        }
    }
    return nullptr;
}

bool iconRow(const char* id, const Entry& entry, bool highlighted, ImGuiSelectableFlags flags = 0) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    bool clicked = ImGui::Selectable(id, highlighted, flags);
    float line = ImGui::GetTextLineHeight();
    drawIcon(ImGui::GetWindowDrawList(), entry.icon, ImVec2(pos.x + 10, pos.y + line / 2), 18.0f);
    ImGui::SetCursorScreenPos(ImVec2(pos.x + 24, pos.y));
    ImGui::TextUnformatted(entry.name.c_str());
    return clicked;
}

void sidebarLink(const Entry& entry, Path target, bool nested) {
    ImGui::PushID(&entry);
    if (nested) {
        ImGui::Indent(LINK_INDENT);
    }
    if (iconRow("##link", entry, path.back() == &entry)) {
        if (path.back() != &entry) {
            go(std::move(target));
        }
    }
    if (nested) {
        ImGui::Unindent(LINK_INDENT);
    }
    ImGui::PopID();
}

void toolbar() {
    ImGui::BeginDisabled(backStack.empty());
    if (ImGui::ArrowButton("back", ImGuiDir_Left)) {
        step(backStack, forwardStack);
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(forwardStack.empty());
    if (ImGui::ArrowButton("forward", ImGuiDir_Right)) {
        step(forwardStack, backStack);
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(path.size() == 1);
    if (ImGui::ArrowButton("up", ImGuiDir_Up)) {
        Path parent(path.begin(), path.end() - 1);
        go(std::move(parent));
    }
    ImGui::EndDisabled();

    std::string address;
    for (const Entry* entry : path) {
        address += address.empty() ? entry->name : " > " + entry->name;
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText("##address", address.data(), address.size() + 1, ImGuiInputTextFlags_ReadOnly);
}

void sidebar() {
    ImGui::BeginChild("sidebar", ImVec2(SIDEBAR_WIDTH, -ImGui::GetTextLineHeightWithSpacing()), ImGuiChildFlags_Borders);
    ImGui::TextDisabled("Quick access");
    for (const char* name : QUICK_ACCESS) {
        if (const Entry* entry = rootChild(name)) {
            ImGui::PushID("quick");
            sidebarLink(*entry, {&root, entry}, true);
            ImGui::PopID();
        }
    }
    ImGui::Spacing();
    sidebarLink(root, {&root}, false);
    for (const Entry& child : root.children) {
        sidebarLink(child, {&root, &child}, true);
    }
    ImGui::EndChild();
}

void contents() {
    const Entry& folder = *path.back();
    float statusBar = ImGui::GetTextLineHeightWithSpacing();
    ImGui::BeginChild("contents", ImVec2(0, -statusBar), ImGuiChildFlags_Borders);
    const Entry* opened = nullptr;

    ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInnerV;
    if (ImGui::BeginTable("files", 4, flags)) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Date modified", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableHeadersRow();

        for (int i = 0; i < static_cast<int>(folder.children.size()); i++) {
            const Entry& entry = folder.children[i];
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushID(i);
            ImGuiSelectableFlags rowFlags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick;
            if (iconRow("##entry", entry, selected == i, rowFlags)) {
                selected = i;
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && entry.icon != Icon::File) {
                    opened = &entry;
                }
            }
            ImGui::PopID();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(entry.modified.c_str());
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(entry.type.c_str());
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(entry.size.c_str());
        }
        ImGui::EndTable();
    }
    if (folder.children.empty()) {
        ImGui::TextDisabled("This folder is empty.");
    }
    ImGui::EndChild();

    if (opened) {
        Path next = path;
        next.push_back(opened);
        go(std::move(next));
    }
}

void statusBar() {
    int count = static_cast<int>(path.back()->children.size());
    ImGui::Text("%d %s", count, count == 1 ? "item" : "items");
    if (selected >= 0) {
        ImGui::SameLine();
        ImGui::TextDisabled("|  1 item selected");
    }
}

}

void draw() {
    toolbar();
    ImGui::BeginGroup();
    sidebar();
    ImGui::SameLine();
    contents();
    ImGui::EndGroup();
    statusBar();
}

}
