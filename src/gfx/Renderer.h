#pragma once

#include <GL/glew.h>
#include <vector>
#include <unordered_map>
#include <SDL_ttf.h>

class Game;
class VertexArray;

class Renderer {
public:
    Renderer(Game* game);
    ~Renderer();

protected:
    void RegisterTexture(std::string path, std::string name);

private:
    void Initialize();
    void InitializeFont();
    void InitializeVertexArrays();

protected:
    Game* mGame;
    TTF_Font* mFont;
    std::unordered_map<const char*, std::unique_ptr<VertexArray>> mVertexArrays;
    std::unordered_map<std::string, GLuint> mTextures;
};