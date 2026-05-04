#include <GL/glew.h>
#include <vector>
#include <unordered_map>
#include <SDL_ttf.h>
#include <glm/glm.hpp>

class Renderer {
public:
    Renderer(class Game* game);
    void Initialize();
    void Draw();

    Game* GetGame() const { return mGame; }
private:
    void DrawCharacter(const glm::vec3 &pos, float scale, const glm::vec4 &fallbackColor,
        const glm::vec3 &up, float yaw, const std::vector<struct LoadedMesh> *meshes,
        const glm::vec4 *colorOverride = nullptr);
    void AddImgInfo(std::string path, std::string name);

private:
    Game* mGame;
    class Shader* mShader;
    TTF_Font* mFont;
    const std::unordered_map<const char*, std::unique_ptr<class VertexArray>>& mVertexArrays;
    std::unordered_map<std::string, GLuint> mTextures;
};