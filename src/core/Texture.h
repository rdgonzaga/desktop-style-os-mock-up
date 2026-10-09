#pragma once

#include <string>

struct Texture {
    unsigned int id = 0;  // 0 means the image couldn't be loaded
    int width = 0;
    int height = 0;
};

Texture loadTexture(const std::string& path);
void freeTexture(Texture& texture);
