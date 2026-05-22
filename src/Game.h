#pragma once

#include <unordered_map>
#include <memory>
#include <vector>
#include <SDL_ttf.h>
#include <GLFW/glfw3.h>

class Actor;
class Player;
class Boat;
class Stage;
class PhysicsSystem;
class MeshLoadSystem;
class UIState;
class ActorLoadSystem;
class CameraSystem;
class MathUtils;
class GameProgressState;
class Renderer3D;
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
    void StartPlayingScene();
    void StartFocusingScene();
    void StartFadeIn();

    void AddActor(std::unique_ptr<Actor> actor);
	void RemoveActor(std::unique_ptr<Actor> actor);
    void RemoveAllActor();
    void AddPlayer(Player* player) { mPlayers.emplace_back(player); };
    void RemoveAllPlayer() { mPlayers.clear(); }
    void SetHitStopTimer(float hitStopTimer) { mHitStopTimer = hitStopTimer; }
    void SetFadeInTimer(float fadeInTimer) { mFadeInTimer = fadeInTimer; }
    void SetCurrentStage(Stage* currentStage) { mCurrentStage = currentStage; }
    void SetCurrentStageNum(int currentStageNum) { mCurrentStageNum = currentStageNum; }
    void SetIsChangeStage(bool isChangeStage) { mIsChangeStage = isChangeStage; }

    GLFWwindow* GetWindow() const { return mWindow; }
    SDL_GameController* GetSdlController() const { return mSdlController;}
    const std::vector<Player*>& GetPlayers() const { return mPlayers; }
    const std::vector<Stage*>& GetStages() const { return mStages; }
    AudioSystem* GetAudioSystem() const { return mAudioSystem.get(); }
    PhysicsSystem* GetPhysicsSystem() const { return mPhysicsSystem.get(); }
    MeshLoadSystem* GetMeshLoadSystem() const { return mMeshLoadSystem.get(); }
    UIState* GetUIState() const { return mUIState.get(); }
    ActorLoadSystem* GetActorLoadSystem() const { return mActorLoadSystem.get(); }
    CameraSystem* GetCameraSystem() const { return mCameraSystem.get(); }
    MathUtils* GetMathUtils() const { return mMathUtils.get(); }
    GameProgressState* GetGameProgressState() const { return mGameProgressState.get(); }
    float GetHitStopTimer() const { return mHitStopTimer; }
    float GetFadeInTimer() const { return mFadeInTimer; }

    Stage* GetCurrentStage() const { return mCurrentStage; }
    int GetCurrentStageNum() const { return mCurrentStageNum; }
    bool GetIsChangeStage() const { return mIsChangeStage; }
    bool GetIsPlayer2Joined() const { return mIsPlayer2Joined; }
    std::string GetCurrentStageYamlPath() const { return mCurrentStageYamlPath; }

private:
    bool InitializeGLFW();
    void InitializeGameController();
    void CreateGameSystems();
    void CreateStages(int stageCount);
    void ReloadCurrentStage();

    void ProcessInput();
    void UpdateGame();
    void GenerateOutput();

    void ProcessGameInput();
    void ProcessActorsInput();
    void UpdateActors(float deltaTime);

    void ChangeStage(int stageNum);
    void CheckGameControllerConnected();

private:
    GLFWwindow* mWindow;
    SDL_GameController* mSdlController;

    std::vector<Player*> mPlayers;
    std::vector<std::unique_ptr<Actor>> mActors;
    std::vector<Stage*> mStages;
    std::vector<std::unique_ptr<Stage>> mStagesUnique;

    std::unique_ptr<AudioSystem> mAudioSystem;
    std::unique_ptr<UIRenderer> mUIRenderer;
    std::unique_ptr<Renderer3D> mRenderer3D;
    std::unique_ptr<PhysicsSystem> mPhysicsSystem;
    std::unique_ptr<CameraSystem> mCameraSystem;
    std::unique_ptr<ActorLoadSystem> mActorLoadSystem;
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
    std::string mCurrentStageYamlPath;
};