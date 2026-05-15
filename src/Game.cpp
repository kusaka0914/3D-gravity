// 学習用にコメントをつけています。
#include <GL/glew.h>
#include "gfx/Shader3D.h"
#include "gfx/UIShader.h"
#include "actor/Planet.h"
#include "Stage.h"
#include "actor/Player.h"
#include "actor/Enemy.h"
#include "gfx/VertexArray.h"
#include "actor/Actor.h"
#include "system/AudioSystem.h"
#include "system/CameraSystem.h"
#include "system/MeshLoadSystem.h"
#include "system/ActorLoadSystem.h"
#include "actor/Key.h"
#include "actor/Boat.h"
#include "actor/Star.h"
#include "actor/Crystal.h"
#include "gfx/UIRenderer.h"
#include "state/UIState.h"
#include "actor/Platform.h"
#include "state/GameProgressState.h"
#include "gfx/Renderer.h"
#include "utils/MathUtils.h"
#include "actor/BoatParts.h"
#include "system/PhysicsSystem.h"
#include "component/DestructibleComponent.h"
#include "component/CollectableComponent.h"
#include "Game.h"
#include "actor/NPC.h"
#include "system/UILoadSystem.h"
#include <GLFW/glfw3.h>
#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <string>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>

Game::Game()
    :mReloadKeyPressedPrev(false)
    ,mUIReloadKeyPressedPrev(false)
    ,mCurrentStageNum(0) 
    ,mIsPlayer2Joined(false)
    ,mHitStopTimer(-1.0f)
    ,mFadeInTimer(-1.0f)
    ,mClearTimer(-1.0f)
    ,mIsChangeStage(false)
    ,mCurrentStagePath("../assets/data/stage0.yaml")
{
    
}

Game::~Game() = default;

bool Game::Initialize()
{
    // glfwの初期化
    if (!glfwInit())
    {
        std::cerr << "Failed to init GLFW" << std::endl;
        return false;
    }

    // OpenGLのバージョンを3.3に設定
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // OpenGLのプロファイルをCoreプロファイルにする
    // 学習用・・・プロファイルは2種類ある（CompatibilityがglBeginなどの古い処理も使える,Coreはシェーダー必須で古いのは使えない）
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // メインのモニタを取得
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    // ゲーム用のウィンドウを作成する
    // mWindow = glfwCreateWindow(mode->width, mode->height, "Engine", monitor, nullptr);
    mWindow = glfwCreateWindow(800, 450, "Engine", nullptr, nullptr);
    if (!mWindow)
    {
        std::cerr << "Failed to create mWindow" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(mWindow);

    // glewの初期化（これがないとOpenGLのシェーダー周りなどの関数が使えない）
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to init GLEW" << std::endl;
        glfwDestroyWindow(mWindow);
        glfwTerminate();
        return false;
    }

    // SDLのゲームパッド用サブシステムを有効にする
    if (SDL_Init(SDL_INIT_GAMECONTROLLER) == 0)
    {
        // SDL_ttfの初期化
        if (TTF_Init() != 0)
            std::cerr << "TTF_Init failed: " << TTF_GetError() << std::endl;
        CheckGameControllerConnected();
    }

    // フォント
    const char *fontPaths[] = {
        "../assets/fonts/NotoSansJP-Black.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
    };
    // 最初に見つかったフォントを用いる
    for (const char *path : fontPaths)
    {
        mFont = TTF_OpenFont(path, 72);
        if (mFont){
            break;
        }
    }

    mAudioSystem = std::make_unique<AudioSystem>(this);
    mShader3D = std::make_unique<Shader3D>();
    mUIShader = std::make_unique<UIShader>();
    mUILoadSystem = std::make_unique<UILoadSystem>(this); 
    mUIRenderer = std::make_unique<UIRenderer>(this);
    mRenderer = std::make_unique<Renderer>(this);
    mUIState = std::make_unique<UIState>(this);
    mMathUtils = std::make_unique<MathUtils>();
    mGameProgressState = std::make_unique<GameProgressState>(this);
    mCameraSystem = std::make_unique<CameraSystem>(this);
    mMeshLoadSystem = std::make_unique<MeshLoadSystem>();
    if (!mShader3D->GetShaderProgram() || !mUIShader->GetShaderProgram())
    {
        glfwTerminate();
        return false;
    }

    for (int i = 0; i< 5; i++) {
        // ステージ作成
        auto stageUnique = std::make_unique<Stage>();
        Stage* stage = stageUnique.get();
        mStagesUnique.emplace_back(std::move(stageUnique));
        mStages.emplace_back(stage);
    }

    mCurrentStage = mStages[0];

    std::vector<float> textLabel = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    };
    mVertexArrays["text"] = std::make_unique<VertexArray>(textLabel.data(), 6, nullptr, 0);

    std::vector<float> hpBar = {
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    };
    mVertexArrays["hpBar"] = std::make_unique<VertexArray>(hpBar.data(), 6, nullptr, 0);

    mActorLoadSystem = std::make_unique<ActorLoadSystem>(this); 

    LoadData(true);

    // 惑星/プレイヤー生成後に行う
    mPhysicsSystem = std::make_unique<PhysicsSystem>(this);

    // 時間情報
    mLastTime = glfwGetTime();

    // 深度テストをONにして奥行きに応じて描画できるようにする（描画順ではなく、手前にあるものが上書きされて描画される）
    glEnable(GL_DEPTH_TEST);

    return true;
}

void Game::RunLoop()
{
    // ゲームループ
    while (!glfwWindowShouldClose(mWindow))
    {
        glfwPollEvents(); // 入力などのイベントを処理する
        ProcessInput();
        UpdateGame();
        GenerateOutput();
    }
}

void Game::Shutdown()
{
    if (mSdlController)
    {
        SDL_GameControllerClose(mSdlController);
    }

    if (mFont) TTF_CloseFont(mFont);
    TTF_Quit();
    SDL_Quit();

    mAudioSystem->Shutdown();

    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

void Game::ProcessInput()
{
    // 入力をSDLに取り込む
    SDL_PumpEvents();
    // ゲームパッド対応
    SDL_GameControllerUpdate();
    for (const auto& actor_unique : mActors) {
        Actor* actor = actor_unique.get();
        actor->ProcessInput();
    }

    mCameraSystem->ProcessInput();

    // データのホットリロード
    bool reloadPressed = (glfwGetKey(mWindow, GLFW_KEY_R) == GLFW_PRESS);
    if (reloadPressed && !mReloadKeyPressedPrev)
    {
        LoadData(true);
        mPhysicsSystem->Initialize();
    }
    mReloadKeyPressedPrev = reloadPressed;

    bool UIreloadPressed = (glfwGetKey(mWindow, GLFW_KEY_U) == GLFW_PRESS);
    if (UIreloadPressed && !mUIReloadKeyPressedPrev)
    {
        mUILoadSystem->Initialize();
    }
    mUIReloadKeyPressedPrev = UIreloadPressed;

    // 2P参加
    bool pKeyNow = (glfwGetKey(mWindow, GLFW_KEY_P) == GLFW_PRESS);
    if (pKeyNow && !mIsPlayer2Joined)
    {
        // 2P作成
        mIsPlayer2Joined = true;
        std::unique_ptr<Player> player2 =std::make_unique<Player>(this);
        Player* player2_ptr = player2.get();
        mActors.emplace_back(std::move(player2));
        mPlayers.emplace_back(player2_ptr);

        // 2Pモデルロード
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

    // ゲーム終了
    if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS || (mSdlController && SDL_GameControllerGetButton(mSdlController, SDL_CONTROLLER_BUTTON_BACK)))
        glfwSetWindowShouldClose(mWindow, GLFW_TRUE);
    mAPressedPrev = aPressed;
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

    for (const auto& actor_unique : mActors) {
        Actor* actor = actor_unique.get();
        actor->Update(deltaTime);
    }

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
        LoadData(true);
        mPhysicsSystem->Initialize();
    }

    AudioSystem* audioSystem = mAudioSystem.get();
    audioSystem->Update();

    mIsChangeStage = false;

    if (mPhysicsSystem)
    {
        mPhysicsSystem->Update();
    }
}

void Game::GenerateOutput()
{
    mRenderer->Draw();
    mUIRenderer->Draw();
    glfwSwapBuffers(mWindow);
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

void Game::LoadData(bool isLoadPlayer) {
    RemoveAllActor();
    mActorLoadSystem->LoadData(isLoadPlayer);
    LoadModel();
}

void Game::LoadModel() {
    // プレイヤーモデルをロード
    for (auto player : mPlayers) {
        auto playerMeshes = mMeshLoadSystem->GetLoadedMeshes("player");
        player->SetMeshes(playerMeshes);
    }
    std::vector<Planet*> planets = mCurrentStage->GetPlanets();
    // 惑星モデルをロード
    for (auto planet : planets)
    {
        auto it = planet->GetModelPath().find(".");
        std::string MeshName = planet->GetModelPath().substr(0, it);
        auto planetMeshes = mMeshLoadSystem->GetLoadedMeshes(MeshName);
        planet->SetMeshes(planetMeshes);
    }
    // 各惑星のオブジェクトのモデルをロード
    for (auto planet : planets) {
        // NPCモデルをロード
        std::vector<NPC*> NPCs = planet->GetNPCs();
        for (auto NPC : NPCs)
        {
            auto it = NPC->GetModelPath().find(".");
            std::string MeshName = NPC->GetModelPath().substr(0, it);
            auto NPCMeshes = mMeshLoadSystem->GetLoadedMeshes(MeshName);
            NPC->SetMeshes(NPCMeshes);
        }

        // 敵モデルをロード
        std::vector<Enemy*> enemies = planet->GetEnemies();
        for (auto enemy : enemies)
        {
            auto it = enemy->GetModelPath().find(".");
            std::string MeshName = enemy->GetModelPath().substr(0, it);
            auto enemyMeshes = mMeshLoadSystem->GetLoadedMeshes(MeshName);
            enemy->SetMeshes(enemyMeshes);
        }

        // 鍵モデルをロード
        Key* key = planet->GetKey();
        if (key) {
            auto keyMeshes = mMeshLoadSystem->GetLoadedMeshes("key");
            key->SetMeshes(keyMeshes);
        }

        // スターモデルをロード
        Star* star = planet->GetStar();
        if (star) {
            auto starMeshes = mMeshLoadSystem->GetLoadedMeshes("star");
            star->SetMeshes(starMeshes);
        }

        // ボートモデルをロード
        std::vector<Boat*> boats = planet->GetBoats();
        if (!boats.empty()) {
            auto boatMeshes = mMeshLoadSystem->GetLoadedMeshes("boat");
            for (auto boat : boats) {
                boat->SetMeshes(boatMeshes);
            }
        }

        // ボートのかけらモデルをロード
        std::vector<BoatParts*> boatParts = planet->GetBoatParts();
        if (!boatParts.empty()) {
            for (auto parts : boatParts) {
                auto it = parts->GetModelPath().find(".");
                std::string MeshName = parts->GetModelPath().substr(0, it);
                auto boatPartsMeshes = mMeshLoadSystem->GetLoadedMeshes(MeshName);
                parts->SetMeshes(boatPartsMeshes);
            }
        }

        // クリスタルモデルをロード
        std::vector<Crystal*> crystals = planet->GetCrystals();
        if (!crystals.empty()) {
            auto crystalsMeshes = mMeshLoadSystem->GetLoadedMeshes("crystals");
            for (auto crystal : crystals) {
                crystal->SetMeshes(crystalsMeshes);
            }
        }

        // クリスタルモデルをロード
        std::vector<Platform*> platforms = planet->GetPlatforms();
        if (!platforms.empty()) {
            auto platformMeshes = mMeshLoadSystem->GetLoadedMeshes("platform");
            for (auto platform : platforms) {
                platform->SetMeshes(platformMeshes);
            }
        }
    }
}

void Game::ChangeStage(int stageNum) {
    mCurrentStage = mStages[stageNum];
    mCurrentStageNum = stageNum;
    mIsChangeStage = true;

    std::string stagePath = "../assets/data/stage" + std::to_string(stageNum) + ".yaml";
    mCurrentStagePath = stagePath;
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
    mPlayers[0]->OnBoatArrived(boat);

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