#include "core/Texture.h"

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture loadTexture(const std::string& path) {
    Texture texture;
    unsigned char* pixels = stbi_load(path.c_str(), &texture.width, &texture.height, nullptr, 4);
    if (!pixels) {
        return texture;
    }

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    stbi_image_free(pixels);
    return texture;
}

void freeTexture(Texture& texture) {
    if (texture.id) {
        glDeleteTextures(1, &texture.id);
    }
    texture = Texture{};
}
