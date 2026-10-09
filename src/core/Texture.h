#pragma once

#include <string>

struct Texture {
    unsigned int id = 0;
    int width = 0;
    int height = 0;
};

Texture loadTexture(const std::string& path);
void freeTexture(Texture& texture);
