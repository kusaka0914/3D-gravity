#include <GL/glew.h>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <SDL_ttf.h>

class UIRenderer {
public:
    UIRenderer(class Game* game);
    void Initialize();
    void Draw();

private:
    void AddImgInfo(std::string path, std::string name);

    void DrawTitle();

    void DrawOpening();

    void DrawOpeningIntro();
    void DrawOpeningTalkWithMother();
    void DrawOpeningTalkWithDoctor();

    void DrawDefaultUI();

    void DrawOperationSupportUI();
    void DrawHpUI();
    void DrawSpecialAttackUI();
    void DrawTalkableUI();
    void DrawRemainPartsUI();

    void DrawStateUI();
   
    void DrawBattleTutorial();
    void DrawBreakTutorial();
    void DrawTalkWithNPC();
    void DrawStageClear();
    void DrawTalkUI(const std::vector<std::string>& texts, int index);
    void DrawLoading();

    void DrawBG(float width, float height, float x, float y, std::vector<GLfloat> color);
    void DrawText(float x, float y, float scale, std::string message, bool isCenterBase, glm::vec4 color =  {255, 255, 255, 255});
    void DrawTexture(float x, float y, float width, float height, std::string textureName);

    Game* GetGame() const { return mGame; }

private:
    Game* mGame;
    class UIShader* mUIShader;
    TTF_Font* mFont;
    class UILoader* mUILoader;

    const std::unordered_map<const char*, std::unique_ptr<class VertexArray>>& mVertexArrays;
    std::unordered_map<std::string, GLuint> mTextures;

    int mFbWidth;
    int mFbHeight;
};