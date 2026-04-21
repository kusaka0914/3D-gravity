#include <GL/glew.h>
#include <vector>
#include <unordered_map>
#include <SDL_ttf.h>

class Renderer {
public:
    Renderer(class Game* game);
    void Draw();

    Game* GetGame() const { return mGame; }
private:
    Game* mGame;
    class Shader* mShader;
    TTF_Font* mFont;
    const std::unordered_map<const char*, std::unique_ptr<class VertexArray>>& mVertexArrays;
};