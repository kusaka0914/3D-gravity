#include "Actor.h"
#include "AudioSystem.h"
#include <map>
#include <unordered_map>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
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

    void AddActor(std::unique_ptr<class Actor> actor) { mActors.emplace_back(std::move(actor)); };
	void RemoveActor(std::unique_ptr<class Actor> actor);
    void RemoveAllActor() { for(int i = 0; i < mActors.size(); i++) mActors.pop_back(); }
    void AddPlayer(class Player* player) { mPlayers.emplace_back(player); };
    void RemoveAllPlayer() { for(int i = 0; i < mPlayers.size(); i++) mPlayers.pop_back(); }
    void SetHitStopTimer(float hitStopTimer) { mHitStopTimer = hitStopTimer; }

    GLFWwindow* GetWindow() const { return mWindow; }
    SDL_GameController* GetSdlController() const { return mSdlController;}
    TTF_Font* GetFont() const { return mFont; }
    const std::unordered_map<const char*, std::unique_ptr<class VertexArray>>& GetVertexArrays() const { return mVertexArrays; }
    const std::vector<std::unique_ptr<class Actor>>& GetActors() const { return mActors; }
    const std::vector<class Player*>& GetPlayers() const { return mPlayers; }
    const std::vector<class Stage*>& GetStages() const { return mStages; }
    AudioSystem* GetAudioSystem() const { return mAudioSystem.get(); }
    class Shader* GetShader() const { return mShader.get(); }
    class PhysicsSystem* GetPhysicsSystem() const { return mPhysicsSystem.get(); }
    class Mesh* GetMesh() const { return mMesh.get(); }
    class UIState* GetUIState() const { return mUIState.get(); }
    float GetHitStopTimer() const { return mHitStopTimer; }

    Stage* GetCurrentStage() const { return mCurrentStage; }
    int GetCurrentStageNum() const { return mCurrentStageNum; }
    bool GetIsStageClear() const { return mIsStageClear; }
    bool GetIsPlayer2Joined() const { return mIsPlayer2Joined; }

private:
    void ProcessInput();
    void UpdateGame();
    void GenerateOutput();
    void LoadData();
    void LoadModel();

    GLFWwindow* mWindow;
    SDL_GameController* mSdlController;
    TTF_Font* mFont;

    std::unordered_map<const char*, std::unique_ptr<class VertexArray>> mVertexArrays;

    std::vector<class Player*> mPlayers;
    std::vector<std::unique_ptr<class Actor>> mActors;
    std::vector<class Stage*> mStages;

    std::unique_ptr<class AudioSystem> mAudioSystem;
    std::unique_ptr<class UIRenderer> mUIRenderer;
    std::unique_ptr<class Renderer> mRenderer;
    std::unique_ptr<class Shader> mShader;
    std::unique_ptr<class PhysicsSystem> mPhysicsSystem;
    std::unique_ptr<class Loader> mLoader;
    std::unique_ptr<class Mesh> mMesh;
    std::unique_ptr<class UIState> mUIState;

    Stage* mCurrentStage;

    int mCurrentStageNum;
    float mHitStopTimer;

    double mLastTime;

    bool mReloadKeyPressedPrev;
    bool mAPressedPrev;
    bool mIsStageClear;
    bool mIsPlayer2Joined;
};