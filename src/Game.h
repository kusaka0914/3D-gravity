#pragma once

#include "actor/Actor.h"
#include "system/AudioSystem.h"
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
    void LoadData(bool isLoadPlayer);
    void ChangeStage(int stageNum);

    void AddActor(std::unique_ptr<class Actor> actor) { mActors.emplace_back(std::move(actor)); };
	void RemoveActor(std::unique_ptr<class Actor> actor);
    void RemoveAllActor() { mPlayers.clear();
        mActors.clear(); }
    void AddPlayer(class Player* player) { mPlayers.emplace_back(player); };
    void RemoveAllPlayer() { mPlayers.clear(); }
    void SetHitStopTimer(float hitStopTimer) { mHitStopTimer = hitStopTimer; }
    void SetFadeInTimer(float fadeInTimer) { mFadeInTimer = fadeInTimer; }
    void SetCurrentStage(class Stage* currentStage) { mCurrentStage = currentStage; }
    void SetCurrentStageNum(int currentStageNum) { mCurrentStageNum = currentStageNum; }
    void SetIsChangeStage(bool isChangeStage) { mIsChangeStage = isChangeStage; }
    void SetCurrentStagePath(std::string currentStagePath) { mCurrentStagePath = currentStagePath; }

    GLFWwindow* GetWindow() const { return mWindow; }
    SDL_GameController* GetSdlController() const { return mSdlController;}
    TTF_Font* GetFont() const { return mFont; }
    const std::unordered_map<const char*, std::unique_ptr<class VertexArray>>& GetVertexArrays() const { return mVertexArrays; }
    const std::vector<std::unique_ptr<class Actor>>& GetActors() const { return mActors; }
    const std::vector<class Player*>& GetPlayers() const { return mPlayers; }
    const std::vector<class Stage*>& GetStages() const { return mStages; }
    AudioSystem* GetAudioSystem() const { return mAudioSystem.get(); }
    class Shader3D* GetShader3D() const { return mShader3D.get(); }
    class UIShader* GetUIShader() const { return mUIShader.get(); }
    class PhysicsSystem* GetPhysicsSystem() const { return mPhysicsSystem.get(); }
    class Mesh* GetMesh() const { return mMesh.get(); }
    class UIState* GetUIState() const { return mUIState.get(); }
    class Loader* GetLoader() const { return mLoader.get(); }
    class UILoader* GetUILoader() const { return mUILoader.get(); }
    class Helper* GetHelper() const { return mHelper.get(); }
    class GameProgressState* GetGameProgressState() const { return mGameProgressState.get(); }
    float GetHitStopTimer() const { return mHitStopTimer; }
    float GetFadeInTimer() const { return mFadeInTimer; }

    Stage* GetCurrentStage() const { return mCurrentStage; }
    int GetCurrentStageNum() const { return mCurrentStageNum; }
    bool GetIsChangeStage() const { return mIsChangeStage; }
    bool GetIsPlayer2Joined() const { return mIsPlayer2Joined; }
    std::string GetCurrentStagePath() const { return mCurrentStagePath; }

private:
    void ProcessInput();
    void UpdateGame();
    void GenerateOutput();
    void LoadModel();

    GLFWwindow* mWindow;
    SDL_GameController* mSdlController;
    TTF_Font* mFont;

    std::unordered_map<const char*, std::unique_ptr<class VertexArray>> mVertexArrays;

    std::vector<class Player*> mPlayers;
    std::vector<std::unique_ptr<class Actor>> mActors;
    std::vector<class Stage*> mStages;
    std::vector<std::unique_ptr<class Stage>> mStagesUnique;

    std::unique_ptr<class AudioSystem> mAudioSystem;
    std::unique_ptr<class UIRenderer> mUIRenderer;
    std::unique_ptr<class Renderer> mRenderer;
    std::unique_ptr<class Shader3D> mShader3D;
    std::unique_ptr<class UIShader> mUIShader;
    std::unique_ptr<class PhysicsSystem> mPhysicsSystem;
    std::unique_ptr<class Loader> mLoader;
    std::unique_ptr<class UILoader> mUILoader;
    std::unique_ptr<class Mesh> mMesh;
    std::unique_ptr<class UIState> mUIState;
    std::unique_ptr<class Helper> mHelper;
    std::unique_ptr<class GameProgressState> mGameProgressState;

    Stage* mCurrentStage;

    int mCurrentStageNum;
    float mHitStopTimer;
    float mFadeInTimer;
    float mClearTimer;

    double mLastTime;

    bool mReloadKeyPressedPrev;
    bool mUIReloadKeyPressedPrev;
    bool mAPressedPrev;
    bool mIsPlayer2Joined;
    bool mIsChangeStage;
    std::string mCurrentStagePath;
};