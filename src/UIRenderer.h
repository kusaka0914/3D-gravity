#include <GL/glew.h>
#include <vector>
#include <unordered_map>
#include <SDL_ttf.h>

class UIRenderer {
public:
    UIRenderer(class Game* game);
    void Draw();

    Game* GetGame() const { return mGame; }
private:
    // void DrawTextBox(float padding, float panelWidth, float panelHeight, float panelX, float panelY, float textScale, std::vector<GLfloat> panelColor, const char* message);
    bool DrawStateUI();
    void DrawDefaultUI();
    void DrawOperationSupportUI();
    void DrawHpUI();
    void DrawBG(float width, float height, float x, float y, std::vector<GLfloat> color);
    void DrawText(float x, float y, float scale, const char* message, bool isCenterBase);
    void DrawTexture(float x, float y, float scale, const char* path);
    
    void DrawTutorial();
    void DrawCrystalTutorial();
    void DrawBattleTutorial();
    void DrawBreakTutorial();
    void DrawStageClear();
    void DrawRemainPartsUI();

    Game* mGame;
    class Shader* mShader;
    TTF_Font* mFont;
    const std::unordered_map<const char*, std::unique_ptr<class VertexArray>>& mVertexArrays;

    int mFbWidth;
    int mFbHeight;
};