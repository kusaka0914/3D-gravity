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

class Actor;
class Player;
class Boat;
class Stage;
class VertexArray;
class Shader3D;
class UIShader;
class PhysicsSystem;
class MeshLoadSystem;
class UIState;
class ActorLoadSystem;
class UILoadSystem;
class CameraSystem;
class MathUtils;
class GameProgressState;
class Renderer;
class UIRenderer;
class AudioSystem;

class Game {
public:
    Game();
    ~Game();
    bool Initialize();
    void RunLoop();
    void Shutdown();
    void LoadData(bool isLoadPlayer);
    void OnBoatStageChangeRequested(int destStage);
    void OnBoatArrived(Boat* boat);
    void OnStarObtained();
    void OnEnemyLaunched();

    void AddActor(std::unique_ptr<Actor> actor) { mActors.emplace_back(std::move(actor)); };
	void RemoveActor(std::unique_ptr<Actor> actor);
    void RemoveAllActor() { mPlayers.clear();
        mActors.clear(); }
    void AddPlayer(Player* player) { mPlayers.emplace_back(player); };
    void RemoveAllPlayer() { mPlayers.clear(); }
    void SetHitStopTimer(float hitStopTimer) { mHitStopTimer = hitStopTimer; }
    void SetFadeInTimer(float fadeInTimer) { mFadeInTimer = fadeInTimer; }
    void SetCurrentStage(Stage* currentStage) { mCurrentStage = currentStage; }
    void SetCurrentStageNum(int currentStageNum) { mCurrentStageNum = currentStageNum; }
    void SetIsChangeStage(bool isChangeStage) { mIsChangeStage = isChangeStage; }
    void SetCurrentStagePath(std::string currentStagePath) { mCurrentStagePath = currentStagePath; }

    GLFWwindow* GetWindow() const { return mWindow; }
    SDL_GameController* GetSdlController() const { return mSdlController;}
    TTF_Font* GetFont() const { return mFont; }
    const std::unordered_map<const char*, std::unique_ptr<VertexArray>>& GetVertexArrays() const { return mVertexArrays; }
    const std::vector<std::unique_ptr<Actor>>& GetActors() const { return mActors; }
    const std::vector<Player*>& GetPlayers() const { return mPlayers; }
    const std::vector<Stage*>& GetStages() const { return mStages; }
    AudioSystem* GetAudioSystem() const { return mAudioSystem.get(); }
    Shader3D* GetShader3D() const { return mShader3D.get(); }
    UIShader* GetUIShader() const { return mUIShader.get(); }
    PhysicsSystem* GetPhysicsSystem() const { return mPhysicsSystem.get(); }
    MeshLoadSystem* GetMeshLoadSystem() const { return mMeshLoadSystem.get(); }
    UIState* GetUIState() const { return mUIState.get(); }
    ActorLoadSystem* GetActorLoadSystem() const { return mActorLoadSystem.get(); }
    UILoadSystem* GetUILoadSystem() const { return mUILoadSystem.get(); }
    CameraSystem* GetCameraSystem() const { return mCameraSystem.get(); }
    MathUtils* GetMathUtils() const { return mMathUtils.get(); }
    GameProgressState* GetGameProgressState() const { return mGameProgressState.get(); }
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
    void ChangeStage(int stageNum);
    void CheckGameControllerConnected();

private:
    GLFWwindow* mWindow;
    SDL_GameController* mSdlController;
    TTF_Font* mFont;

    std::unordered_map<const char*, std::unique_ptr<VertexArray>> mVertexArrays;

    std::vector<Player*> mPlayers;
    std::vector<std::unique_ptr<Actor>> mActors;
    std::vector<Stage*> mStages;
    std::vector<std::unique_ptr<Stage>> mStagesUnique;

    std::unique_ptr<AudioSystem> mAudioSystem;
    std::unique_ptr<UIRenderer> mUIRenderer;
    std::unique_ptr<Renderer> mRenderer;
    std::unique_ptr<Shader3D> mShader3D;
    std::unique_ptr<UIShader> mUIShader;
    std::unique_ptr<PhysicsSystem> mPhysicsSystem;
    std::unique_ptr<CameraSystem> mCameraSystem;
    std::unique_ptr<ActorLoadSystem> mActorLoadSystem;
    std::unique_ptr<UILoadSystem> mUILoadSystem;
    std::unique_ptr<MeshLoadSystem> mMeshLoadSystem;
    std::unique_ptr<UIState> mUIState;
    std::unique_ptr<MathUtils> mMathUtils;
    std::unique_ptr<GameProgressState> mGameProgressState;

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