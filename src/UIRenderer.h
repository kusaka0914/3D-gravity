#include <GL/glew.h>
#include <vector>
#include <unordered_map>
#include <SDL_ttf.h>

class UIRenderer {
public:
    UIRenderer(class Game* game);
    void Initialize();
    void Draw();

    Game* GetGame() const { return mGame; }
private:
    void AddImgInfo(std::string path, std::string name);

    void DrawTitle();
    bool DrawStateUI();
    void DrawDefaultUI();
    void DrawOperationSupportUI();
    void DrawHpUI();
    void DrawSpecialAttackUI();
    void DrawBG(float width, float height, float x, float y, std::vector<GLfloat> color);
    void DrawText(float x, float y, float scale, const char* message, bool isCenterBase);
    void DrawTexture(float x, float y, float width, float height, std::string textureName);
    
    void DrawTutorial();
    void DrawCrystalTutorial();
    void DrawBattleTutorial();
    void DrawBreakTutorial();
    void DrawStageClear();
    void DrawRemainPartsUI();

private:
    Game* mGame;
    class Shader* mShader;
    TTF_Font* mFont;
    const std::unordered_map<const char*, std::unique_ptr<class VertexArray>>& mVertexArrays;
    std::unordered_map<std::string, GLuint> mTextures;

    int mFbWidth;
    int mFbHeight;
};