#include "Renderer.h"
#include <GL/glew.h>
#include <SDL_ttf.h>
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

class Game;
class Enemy;
class Shader3D;
class VertexArray;
class Player;

class Renderer3D : public Renderer {
public:
    Renderer3D(Game* game);
    ~Renderer3D();
    void Initialize();
    void Draw();

private:
    void DrawScene(const glm::mat4& viewMat, const glm::mat4& projMat);
    void DrawCharacter(const glm::vec3& pos, glm::vec3 scale, const glm::vec4& fallbackColor, const glm::vec3& up,
                       float yaw, const std::vector<struct LoadedMesh>* Meshes,
                       const glm::vec4* colorOverride = nullptr);
    void DrawAttackRange(Player* player);
    void DrawAttackRangeVertices(const std::vector<glm::vec3>& vertices, GLenum drawMode, const glm::vec4& color);
    void DrawGuard(glm::mat4 viewMat, Enemy* enemy);
    std::vector<glm::mat4> GetViews();

private:
    std::unique_ptr<Shader3D> mShader3DUnique;
    Shader3D* mShader3D;
    GLuint mAttackRangeVAO = 0;
    GLuint mAttackRangeVBO = 0;
};