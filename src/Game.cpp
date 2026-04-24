// 学習用にコメントをつけています。
#include <GL/glew.h>
#include "Shader.h"
#include "Planet.h"
#include "Stage.h"
#include "Player.h"
#include "Enemy.h"
#include "VertexArray.h"
#include "Actor.h"
#include "AudioSystem.h"
#include "Mesh.h"
#include "Loader.h"
#include "Key.h"
#include "Boat.h"
#include "Star.h"
#include "Crystal.h"
#include "UIRenderer.h"
#include "UIState.h"
#include "GameProgressState.h"
#include "Renderer.h"
#include "BoatParts.h"
#include "PhysicsSystem.h"
#include "DestructibleComponent.h"
#include "Game.h"
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
    ,mCurrentStageNum(0) 
    ,mIsPlayer2Joined(false)
    ,mHitStopTimer(-1.0f)
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
    mWindow = glfwCreateWindow(800, 600, "Engine", nullptr, nullptr);
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
        // コントローラー接続
        for (int i = 0; i < SDL_NumJoysticks(); ++i)
        {
            if (SDL_IsGameController(i))
            {
                mSdlController = SDL_GameControllerOpen(i);
            }
        }
    }

    // フォント
    const char *fontPaths[] = {
        "../assets/fonts/NotoSansJP-Black.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
    };
    // 最初に見つかったフォントを用いる
    for (const char *path : fontPaths)
    {
        mFont = TTF_OpenFont(path, 24);
        if (mFont){
            std::cout << path << std::endl;
            break;
        }
    }

    mAudioSystem = std::make_unique<AudioSystem>(this);
    mShader = std::make_unique<Shader>();
    mUIRenderer = std::make_unique<UIRenderer>(this);
    mRenderer = std::make_unique<Renderer>(this);
    mUIState = std::make_unique<UIState>(this);
    mGameProgressState = std::make_unique<GameProgressState>(this);
    if (!mShader->GetShaderProgram())
    {
        glfwTerminate();
        return false;
    }

    // ステージ作成
    auto stageUnique = std::make_unique<Stage>(this);
    Stage* stage = stageUnique.get();
    mActors.emplace_back(std::move(stageUnique));
    mStages.emplace_back(stage);
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

    mLoader = std::make_unique<Loader>(this); 

    LoadData();

    // 惑星/プレイヤー生成後に行う
    mPhysicsSystem = std::make_unique<PhysicsSystem>(this);
    mMesh = std::make_unique<Mesh>();

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
    // for (auto &p : textTextureCache)
    //     glDeleteTextures(1, &p.second.first);
    if (mFont) TTF_CloseFont(mFont);
    TTF_Quit();
    SDL_Quit();

    mAudioSystem->Shutdown();

    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

void Game::ProcessInput()
{
    for (const auto& actor_unique : mActors) {
        Actor* actor = actor_unique.get();
        actor->ProcessInput();
    }

    // データのホットリロード
    bool reloadPressed = (glfwGetKey(mWindow, GLFW_KEY_R) == GLFW_PRESS);
    if (reloadPressed && !mReloadKeyPressedPrev)
    {
        LoadData();
    }
    mReloadKeyPressedPrev = reloadPressed;

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
        std::string path = "../assets/models/player.obj";
        std::vector<LoadedMesh> playerMeshes = mMesh->loadMeshFromFile(path.c_str());
        mPlayers[1]->SetMeshes(playerMeshes);
    } 

    bool aPressed = SDL_GameControllerGetButton(mSdlController, SDL_CONTROLLER_BUTTON_A);
    if (aPressed && !mAPressedPrev) {
        if (mUIState->GetIsTutorialActive()) {
            mUIState->SetIsTutorialActive(false);
            mUIState->SetIsCrystalTutorialActive(true);
        } else if (mUIState->GetIsCrystalTutorialActive()) {
            mUIState->SetIsCrystalTutorialActive(false);
            mUIState->SetIsUIActive(false);
            mPlayers[0]->SetCanMove(true);
        }
        if (mUIState->GetIsBattleTutorialActive()) {
            mUIState->SetIsBattleTutorialActive(false);
            mUIState->SetIsUIActive(false);
            mPlayers[0]->SetCanMove(true);
        }
        if (mUIState->GetIsBreakTutorialActive()) {
            mUIState->SetIsBreakTutorialActive(false);
            mUIState->SetIsUIActive(false);
            mPlayers[0]->SetCanMove(true);
        }
    }

    // ゲーム終了
    if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS || (mSdlController && SDL_GameControllerGetButton(mSdlController, SDL_CONTROLLER_BUTTON_BACK)))
        glfwSetWindowShouldClose(mWindow, GLFW_TRUE);
    mAPressedPrev = aPressed;
}

void Game::UpdateGame()
{
    double currentTime = glfwGetTime(); 
    float deltaTime = std::min(0.04f, static_cast<float>(currentTime - mLastTime));
    if (mHitStopTimer >= 0.0f) {
        mHitStopTimer-= deltaTime;
        deltaTime = 0.0f;
    }
    if (mUIState->GetIsUIActive()) {
        deltaTime = 0.0f;
    }

    mLastTime = currentTime;

    for (const auto& actor_unique : mActors) {
        Actor* actor = actor_unique.get();
        actor->Update(deltaTime);
    }

    AudioSystem* audioSystem = mAudioSystem.get();
    audioSystem->Update();

    if (mPhysicsSystem)
    {
        mPhysicsSystem->Update();
    }

    for (int i = 0; i < SDL_NumJoysticks(); ++i)
    {
        
        if (SDL_IsGameController(i))
        {
            mSdlController = SDL_GameControllerOpen(i);
        }
    }
}

void Game::GenerateOutput()
{
    mRenderer->Draw();
    mUIRenderer->Draw();
    // バッファーを入れ替える
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

void Game::LoadData() {
    mLoader->LoadDataFromYaml();
    LoadModel();
}

void Game::LoadModel() {
    // プレイヤーモデルをロード
    for (auto player : mPlayers) {
        std::string path = "../assets/models/player.obj";
        std::vector<LoadedMesh> playerMeshes = mMesh->loadMeshFromFile(path.c_str());
        player->SetMeshes(playerMeshes);
    }
    std::vector<Planet*> planets = mCurrentStage->GetPlanets();
    // 惑星モデルをロード
    for (auto planet : planets)
    {
        std::unordered_map<std::string, std::vector<LoadedMesh>> planetMeshesByPath = mCurrentStage->GetPlanetMeshesByPath();
        if (planetMeshesByPath.find(planet->GetModelPath()) == planetMeshesByPath.end())
        {
            std::string path = "../assets/models/" + planet->GetModelPath();
            auto planetMeshes = mMesh->loadMeshFromFile(path.c_str());
            mCurrentStage->AddPlanetMesh(planet->GetModelPath(), planetMeshes);
        }
    }
    // 各惑星のオブジェクトのモデルをロード
    for (auto planet : planets) {
        // 敵モデルをロード
        std::vector<Enemy*> enemies = planet->GetEnemies();
        for (auto enemy : enemies)
        {
            std::unordered_map<std::string, std::vector<LoadedMesh>> enemyMeshesByPath = mCurrentStage->GetPlanets()[0]->GetEnemyMeshesByPath();
            std::string path = "../assets/models/" + enemy->GetModelPath();
            auto enemyMeshes = mMesh->loadMeshFromFile(path.c_str());
            mCurrentStage->GetPlanets()[0]->AddEnemyMesh(enemy->GetModelPath(), enemyMeshes);
        }

        // 鍵モデルをロード
        Key* key = planet->GetKey();
        if (key) {
            std::vector<LoadedMesh> keyMeshes = mMesh->loadMeshFromFile("../assets/models/key.obj");
            key->SetMeshes(keyMeshes);
        }

        // スターモデルをロード
        Star* star = planet->GetStar();
        if (star) {
            std::vector<LoadedMesh> starMeshes = mMesh->loadMeshFromFile("../assets/models/star.obj");
            star->SetMeshes(starMeshes);
        }

        // ボートモデルをロード
        std::vector<Boat*> boats = planet->GetBoats();
        if (!boats.empty()) {
            std::vector<LoadedMesh> boatMeshes = mMesh->loadMeshFromFile("../assets/models/boat.obj");
            for (auto boat : boats) {
                boat->SetMeshes(boatMeshes);
            }
        }

        // ボートのかけらモデルをロード
        std::vector<BoatParts*> boatParts = planet->GetBoatParts();
        if (!boatParts.empty()) {
            for (auto parts : boatParts) {
                std::string path = "../assets/models/" + parts->GetModelPath();
                std::vector<LoadedMesh> boatPartsMeshes = mMesh->loadMeshFromFile(path.c_str());
                parts->SetMeshes(boatPartsMeshes);
            }
        }

        // クリスタルモデルをロード
        std::vector<Crystal*> crystals = planet->GetCrystals();
        if (!crystals.empty()) {
            std::vector<LoadedMesh> crystalsMeshes = mMesh->loadMeshFromFile("../assets/models/crystals.fbx");
            for (auto parts : crystals) {
                parts->SetMeshes(crystalsMeshes);
            }
        }
    }
}