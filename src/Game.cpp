#include <GL/glew.h>
#include "Game.h"
#include "Stage.h"
#include "actor/Planet.h"
#include "actor/Player.h"
#include "actor/Actor.h"
#include "actor/NPC.h"
#include "system/AudioSystem.h"
#include "system/CameraSystem.h"
#include "system/MeshLoadSystem.h"
#include "system/ActorLoadSystem.h"
#include "system/PhysicsSystem.h"
#include "system/UILoadSystem.h"
#include "gfx/UIRenderer.h"
#include "gfx/Renderer3D.h"
#include "state/UIState.h"
#include "state/GameProgressState.h"
#include "utils/MathUtils.h"

Game::Game()
    :mReloadKeyPressedPrev(false)
    , mUIReloadKeyPressedPrev(false)
    , mCurrentStageNum(0) 
    , mIsPlayer2Joined(false)
    , mHitStopTimer(-1.0f)
    , mFadeInTimer(-1.0f)
    , mClearTimer(-1.0f)
    , mIsChangeStage(false)
    , mCurrentStageYamlPath("../assets/data/stage0.yaml")
{
    
}

Game::~Game() = default;

bool Game::Initialize() {
    if (!InitializeGLFW()) return false;

    InitializeGameController();
    CreateGameSystems();

    constexpr int stageCount = 5;
    CreateStages(stageCount);

    ReloadCurrentStage();

    mLastTime = glfwGetTime();
    glEnable(GL_DEPTH_TEST);

    return true;
}

bool Game::InitializeGLFW() {
    if (!glfwInit())
    {
        std::cerr << "Failed to init GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    // mWindow = glfwCreateWindow(mode->width, mode->height, "Engine", monitor, nullptr);
    mWindow = glfwCreateWindow(800, 450, "Engine", nullptr, nullptr);
    if (!mWindow)
    {
        std::cerr << "Failed to create mWindow" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(mWindow);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to init GLEW" << std::endl;
        glfwDestroyWindow(mWindow);
        glfwTerminate();
        return false;
    }
    return true;
}

void Game::InitializeGameController() {
    if (SDL_Init(SDL_INIT_GAMECONTROLLER) == 0)
        CheckGameControllerConnected();
}

void Game::CreateGameSystems() {
    mAudioSystem = std::make_unique<AudioSystem>(this);
    mUIRenderer = std::make_unique<UIRenderer>(this);
    mRenderer3D = std::make_unique<Renderer3D>(this);
    mUIState = std::make_unique<UIState>(this);
    mMathUtils = std::make_unique<MathUtils>();
    mGameProgressState = std::make_unique<GameProgressState>(this);
    mCameraSystem = std::make_unique<CameraSystem>(this);
    mMeshLoadSystem = std::make_unique<MeshLoadSystem>(this);
    mActorLoadSystem = std::make_unique<ActorLoadSystem>(this);
    mPhysicsSystem = std::make_unique<PhysicsSystem>(this);
}

void Game::CreateStages(int stageCount) {
    for (int i = 0; i < stageCount; i++) {
        auto stageUnique = std::make_unique<Stage>();
        Stage* stage = stageUnique.get();
        mStagesUnique.emplace_back(std::move(stageUnique));
        mStages.emplace_back(stage);
        if (i == 0)
            mCurrentStage = stage;
    }
}

void Game::ReloadCurrentStage() {
    LoadData(true);
    mPhysicsSystem->Initialize();
    mAudioSystem->TryChangeBGM();
}

void Game::RunLoop()
{
    while (!glfwWindowShouldClose(mWindow))
    {
        glfwPollEvents();
        ProcessInput();
        UpdateGame();
        GenerateOutput();
    }
}

void Game::Shutdown()
{
    if (mSdlController)
        SDL_GameControllerClose(mSdlController);

    SDL_Quit();

    mAudioSystem->Shutdown();

    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

void Game::ProcessInput()
{
    SDL_PumpEvents();
    SDL_GameControllerUpdate();
    
    ProcessGameInput();
    ProcessActorsInput();
    mCameraSystem->ProcessInput();
}

void Game::ProcessGameInput() {
    bool reloadPressed = (glfwGetKey(mWindow, GLFW_KEY_R) == GLFW_PRESS);
    if (reloadPressed && !mReloadKeyPressedPrev) {
        ReloadCurrentStage();
    }
    mReloadKeyPressedPrev = reloadPressed;

    bool UIreloadPressed = (glfwGetKey(mWindow, GLFW_KEY_U) == GLFW_PRESS);
    if (UIreloadPressed && !mUIReloadKeyPressedPrev) {
        mUIRenderer->GetUILoadSystem()->Initialize();
    }
    mUIReloadKeyPressedPrev = UIreloadPressed;

    bool pKeyNow = (glfwGetKey(mWindow, GLFW_KEY_P) == GLFW_PRESS);
    if (pKeyNow && !mIsPlayer2Joined) {
        mIsPlayer2Joined = true;
        std::unique_ptr<Player> player2 =std::make_unique<Player>(this);
        Player* player2_ptr = player2.get();
        mActors.emplace_back(std::move(player2));
        mPlayers.emplace_back(player2_ptr);

        auto playerMeshes = mMeshLoadSystem->GetLoadedMeshes("player");
        mPlayers[1]->SetMeshes(playerMeshes);
    } 

    bool aPressed = SDL_GameControllerGetButton(mSdlController, SDL_CONTROLLER_BUTTON_A);
    if (aPressed && !mAPressedPrev) {
        bool isTitle = mGameProgressState->GetSceneState() == GameProgressState::SceneState::Title;
        bool isOpening = mGameProgressState->GetSceneState() == GameProgressState::SceneState::Opening;
        bool isTalking = mGameProgressState->GetSceneState() == GameProgressState::SceneState::Talking;
        if (isTitle && mFadeInTimer <= -1.0f) {
            mFadeInTimer = 1.0f;
            mGameProgressState->SetNextSceneState(GameProgressState::SceneState::Opening);
        } 
        if (isTalking || isOpening) {
            mUIState->IncTalkUIIndex();
        }
        std::vector<NPC*> NPCs = mPlayers[0]->GetCurrentPlanet()->GetNPCs();
        for (auto NPC : NPCs) {
            bool isTalkable = NPC->GetIsTalkable();
            if (isTalkable) {
                mUIState->SetCurrentTalkWith(UIState::TalkWith::NPC);
                mPlayers[0]->SetTalkingNPC(NPC);
                mGameProgressState->SetCurrentSceneState(GameProgressState::SceneState::Talking);
            }
        }
    }

    if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS || (mSdlController && SDL_GameControllerGetButton(mSdlController, SDL_CONTROLLER_BUTTON_BACK)))
        glfwSetWindowShouldClose(mWindow, GLFW_TRUE);
    
    mAPressedPrev = aPressed;
}

void Game::ProcessActorsInput() {
    bool isPlaying = mGameProgressState->GetSceneState() == GameProgressState::SceneState::Playing;
    if(!isPlaying) return;
    
    for (const auto& actor_unique : mActors) {
        Actor* actor = actor_unique.get();
        actor->ProcessInput();
    }
}

void Game::UpdateGame()
{
    CheckGameControllerConnected();

    double currentTime = glfwGetTime(); 
    float deltaTime = std::min(0.04f, static_cast<float>(currentTime - mLastTime));
    mLastTime = currentTime;

    if (mFadeInTimer > -1.0f) {
        float prevFadeInTimer = mFadeInTimer;
        mFadeInTimer -= deltaTime;
        GameProgressState::SceneState nextSceneState = mGameProgressState->GetNextSceneState();
        if (prevFadeInTimer >= 0.0f && mFadeInTimer <= 0.0f) {
            switch (nextSceneState)
            {
            case GameProgressState::SceneState::Opening:
                mGameProgressState->SetCurrentSceneState(GameProgressState::SceneState::Opening);
                mGameProgressState->SetNextSceneState(GameProgressState::SceneState::None);
                break;

            case GameProgressState::SceneState::Playing:
                mGameProgressState->SetCurrentSceneState(GameProgressState::SceneState::Playing);
                mGameProgressState->SetNextSceneState(GameProgressState::SceneState::None);
                ChangeStage(0);
                mUIState->SetCurrentTalkWith(UIState::TalkWith::None);
                break;
            
            default:
                break;
            }
        }
    }
    if (mHitStopTimer >= 0.0f) {
        mHitStopTimer -= deltaTime;
        deltaTime = 0.0f;
    }
    if (mGameProgressState->GetSceneState() == GameProgressState::SceneState::ShowUI) {
        deltaTime = 0.0f;
    }

    bool isTitle = mGameProgressState->GetSceneState() == GameProgressState::SceneState::Title;
    bool isTalking = mGameProgressState->GetSceneState() == GameProgressState::SceneState::Talking;
    bool isShowUI = mGameProgressState->GetSceneState() == GameProgressState::SceneState::ShowUI;
    if (isTitle || isTalking || isShowUI)
        return;

    UpdateActors(deltaTime);

    mCameraSystem->Update(deltaTime);

    if (mClearTimer >= 0.0f) {
        mClearTimer -= deltaTime;
        if (mClearTimer < 0.0f) {
            mFadeInTimer = 1.0f;
            mGameProgressState->SetNextSceneState(GameProgressState::SceneState::Playing);
        }
    }   

    if (mFadeInTimer >= 0.0f) return;

    if (mIsChangeStage) {
        Mix_HaltMusic();
        ReloadCurrentStage();
        mIsChangeStage = false;
    }

    AudioSystem* audioSystem = mAudioSystem.get();
    audioSystem->Update();
}

void Game::UpdateActors(float deltaTime) {
    for (const auto& actor_unique : mActors) {
        Actor* actor = actor_unique.get();
        actor->Update(deltaTime);
    }
}

void Game::GenerateOutput() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    mUIRenderer->DrawSkyBox();
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    
    mRenderer3D->Draw();
    mUIRenderer->Draw();

    glfwSwapBuffers(mWindow);
}

void Game::AddActor(std::unique_ptr<Actor> actor) { 
    mActors.emplace_back(std::move(actor)); 
}

void Game::RemoveActor(std::unique_ptr<Actor> actor)
{
    auto iter = std::find(mActors.begin(), mActors.end(), actor);
    if (iter != mActors.end())
    {
        std::iter_swap(iter, mActors.end() - 1);
        mActors.pop_back();
    }
}

void Game::RemoveAllActor() {
    mPlayers.clear();
    mActors.clear();
}

void Game::LoadData(bool isLoadPlayer) {
    RemoveAllActor();
    mActorLoadSystem->LoadData(isLoadPlayer);
    mMeshLoadSystem->LoadModel();
}

void Game::ChangeStage(int stageNum) {
    mCurrentStage = mStages[stageNum];
    mCurrentStageNum = stageNum;
    mIsChangeStage = true;
    mCurrentStageYamlPath = "../assets/data/stage" + std::to_string(stageNum) + ".yaml";
}

void Game::CheckGameControllerConnected() {
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i))
            mSdlController = SDL_GameControllerOpen(i);
    }
}

void Game::OnBoatStageChangeRequested(int destStage) {
    if (mCurrentStageNum != 0) return;

    ChangeStage(destStage);
    mFadeInTimer = 1.0f;
}

void Game::OnBoatArrived(Boat* boat) {
    std::vector<Planet*> planets = mCurrentStage->GetPlanets();

    for (auto player : mPlayers) {
        player->SetCurrentPlanet(planets[player->GetCurrentPlanetNum() + 1]);
        player->OnBoatArrived(boat);
    }

    if (mUIState->GetIsBattleTutorialShown()) return;
            
    mUIState->SetCurrentTutorialKind(UIState::TutorialKind::Battle);
    mGameProgressState->SetCurrentSceneState(GameProgressState::SceneState::Talking);
    mUIState->SetIsBattleTutorialShown(true);
}

void Game::OnStarObtained() {
    mGameProgressState->SetCurrentSceneState(GameProgressState::SceneState::StageClear);
    Mix_HaltMusic();
    mAudioSystem->PlaySE("clearSE");
    mClearTimer = 12.0f;
}

void Game::OnEnemyLaunched() {
    if (mGameProgressState->GetIsFirstBreak()) return;
    
    mGameProgressState->SetIsFirstBreak(true);
    mUIState->SetCurrentTutorialKind(UIState::TutorialKind::Break);
    mGameProgressState->SetCurrentSceneState(GameProgressState::SceneState::Talking);
}

void Game::StartPlayingScene() {
    mGameProgressState->SetCurrentSceneState(GameProgressState::SceneState::Playing);
}

void Game::StartFocusingScene() {
    mGameProgressState->SetCurrentSceneState(GameProgressState::SceneState::Focusing);
}

void Game::StartFadeIn() {
    mFadeInTimer = 1.0f;
    mGameProgressState->SetNextSceneState(GameProgressState::SceneState::Playing);
    mUIState->OnFadeIn();
}