#include <map>
#include <GLFW/glfw3.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <memory>
#include <vector>

class Game {
public:
    bool Initialize();
    void RunLoop();
    void Shutdown();

    void AddActor(std::unique_ptr<class Actor> actor);
	void RemoveActor(std::unique_ptr<class Actor> actor);
private:
    void ProcessInput();
    void UpdateGame();
    void GenerateOutput();

    GLFWwindow* mWindow;
    SDL_GameController* mSdlController;
    TTF_Font* mFont;
    std::unorderd_map<const char*, Mix_Music*> mBGMList;
    std::unorderd_map<const char*, Mix_Music*> mSEList;
    std::unorderd_map<const char*, std::unique_ptr<class VertexArray>> mVertexArrays;
    std::vector<std::unique_ptr<class Actor>> mActors;
    std::vector<std::unique_ptr<class Stage>> mStages;
    int mCurrentStage;

    bool mReloadKeyPressedPrev;
};