#include <GL/glew.h>
#include <vector>
#include <unordered_map>
#include <SDL_ttf.h>

class UIRenderer {
public:
    UIRenderer(class Game* game);
    void Draw();
    void DrawTextBox(int fbWidth, int fbHeight, float padding, float panelWidth, float panelHeight, float panelX, float panelY, std::vector<GLfloat> panelColor, const char* message);
    void DrawPanel(int fbWidth, int fbHeight, float panelWidth, float panelHeight, float panelX, float panelY, std::vector<GLfloat> panelColor);
    void DrawText(int fbWidth, int fbHeight, float textX, float textY, const char* message);

    Game* GetGame() const { return mGame; }
private:
    Game* mGame;
    class Shader* mShader;
    TTF_Font* mFont;
    const std::unordered_map<const char*, std::unique_ptr<class VertexArray>>& mVertexArrays;
};