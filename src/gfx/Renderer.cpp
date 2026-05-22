#include "Renderer.h"
#include "VertexArray.h"
#include "Game.h"
#include "thirdParty/stb_image.h"
#include <iostream>


Renderer::Renderer(Game* game)
    : mGame(game)
{
    Initialize();
}

Renderer::~Renderer() {
    if (mFont) TTF_CloseFont(mFont);
    TTF_Quit();
}

void Renderer::Initialize() {
    InitializeFont();
    InitializeVertexArrays();
}

void Renderer::InitializeFont() {
    if (TTF_Init() != 0)
        std::cerr << "TTF_Init failed: " << TTF_GetError() << std::endl;

    const char *fontPaths[] = {
        "../assets/fonts/NotoSansJP-Black.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
    };

    for (const char *path : fontPaths) {
        mFont = TTF_OpenFont(path, 72);
        if (mFont) break;
    }
}

void Renderer::InitializeVertexArrays() {
    std::vector<float> textLabel = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    };
    mVertexArrays["text"] = std::make_unique<VertexArray>(textLabel.data(), 6, nullptr, 0);

    std::vector<float> hpBar = {
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    };
    mVertexArrays["hpBar"] = std::make_unique<VertexArray>(hpBar.data(), 6, nullptr, 0);
}

void Renderer::RegisterTexture(std::string path, std::string name) {
    int imgWidth, imgHeight, imgChannels;
    unsigned char* imageData = stbi_load(path.c_str(), &imgWidth, &imgHeight, &imgChannels, STBI_rgb_alpha);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    stbi_image_free(imageData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    mTextures[name] = tex;
}