#include "Actor.h"
#include <map>
#include <unordered_map>
#include <GLFW/glfw3.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <memory>
#include <vector>
#include <SDL_ttf.h>

class Game {
public:
    Game();
    ~Game();
    bool Initialize();
    void RunLoop();
    void Shutdown();

    void AddActor(std::unique_ptr<class Actor> actor);
	void RemoveActor(std::unique_ptr<class Actor> actor);

    const std::vector<std::unique_ptr<class Actor>>& GetActors() const { return mActors; }
    const std::vector<class Player*>& GetPlayers() const { return mPlayers; }

    const std::unordered_map<const char*, Mix_Music*>& GetBGMList() const { return mBGMList; }
    const std::unordered_map<const char*, Mix_Chunk*>& GetSEList() const { return mSEList; }
    const std::vector<class Stage*>& GetStages() const { return mStages; }
    int GetCurrentStageNum() const { return mCurrentStageNum; }
    bool GetIsStageClear() const { return mIsStageClear; }

private:
    void ProcessInput();
    void UpdateGame();
    void GenerateOutput();

    GLFWwindow* mWindow;
    SDL_GameController* mSdlController;
    TTF_Font* mFont;
    std::unordered_map<const char*, Mix_Music*> mBGMList;
    std::unordered_map<const char*, Mix_Chunk*> mSEList;
    std::unordered_map<const char*, std::unique_ptr<class VertexArray>> mVertexArrays;
    std::vector<class Player*> mPlayers;
    std::vector<std::unique_ptr<class Actor>> mActors;
    std::vector<class Stage*> mStages;
    std::unique_ptr<class AudioSystem> mAudioSystem;
    int mCurrentStageNum;
    double mLastTime;

    bool mReloadKeyPressedPrev;
    bool mIsStageClear;
};