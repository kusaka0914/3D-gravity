#include "Renderer.h"
#include <GL/glew.h>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <SDL_ttf.h>

class Game;
class UIShader;
class UILoadSystem;
class VertexArray;

class UIRenderer : public Renderer {
public:
    UIRenderer(Game* game);
    ~UIRenderer();
    void Draw();
    void DrawSkyBox();

    UILoadSystem* GetUILoadSystem() const { return mUILoadSystem; }

private:
    void Initialize();
    void RegisterUITextures();

    void DrawTitle();
    void DrawOpening();
    void DrawGameOver();

    void DrawOpeningIntro();
    void DrawOpeningTalkWithMother();
    void DrawOpeningTalkWithDoctor();

    void DrawDefaultUI();

    void DrawOperationSupportUI();
    void DrawHpUI();
    void DrawJewelUI();
    void DrawTalkableUI();
    void DrawRemainPartsUI();

    void DrawStateUI();
   
    void DrawBattleTutorial();
    void DrawBreakTutorial();
    void DrawSpecialAttackTutorial();
    void DrawTalkWithNPC();
    void DrawStageClear();
    void DrawTalkUI(const std::vector<std::string>& texts, int index);
    void DrawLoading();

    void DrawBG(float width, float height, float x, float y, std::vector<GLfloat> color);
    void DrawText(float x, float y, float scale, std::string message, bool isCenterBase, glm::vec4 color =  {255, 255, 255, 255});
    void DrawTexture(float x, float y, float width, float height, std::string textureName);

private:
    std::unique_ptr<UIShader> mUIShaderUnique;
    UIShader* mUIShader;
    std::unique_ptr<UILoadSystem> mUILoadSystemUnique;
    UILoadSystem* mUILoadSystem;

    int mFbWidth;
    int mFbHeight;
};