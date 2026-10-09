#pragma once

#include <string>
#include <vector>

#include "core/Icons.h"

namespace taskmanager {

struct Process {
    std::string name;
    Icon icon;
    bool suspended;
    float cpu;
    float memory;
    float disk;
    float network;
};

std::vector<Process> processes();
void draw();

}
