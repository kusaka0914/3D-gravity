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
#include "UIRenderer.h"
#include "BoatParts.h"
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
    mWindow = glfwCreateWindow(mode->width, mode->height, "Engine", monitor, nullptr);
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

    // コントローラー接続
    mSdlController = nullptr;
    // SDLのゲームパッド用サブシステムを有効にする
    if (SDL_Init(SDL_INIT_GAMECONTROLLER) == 0)
    {
        // SDL_ttfの初期化
        if (TTF_Init() != 0)
            std::cerr << "TTF_Init failed: " << TTF_GetError() << std::endl;
        // 今つながっているコントローラーの数を取得してループ
        for (int i = 0; i < SDL_NumJoysticks(); ++i)
        {
            // i番目がゲームコントローラーとして認識できるか判定
            if (SDL_IsGameController(i))
            {
                // それをゲームコントローラーとして開く
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
    // 惑星をYAMLから読み込み
    if (!mLoader->loadPlanetsFromYaml("../assets/data/planets.yaml")) {
        std::cerr << "Planet YAML load failed" << std::endl;
    }
    std::vector<Planet*> planets = mCurrentStage->GetPlanets();
    for (auto planet : planets)
    {
        planet->Initialize();
    }
    // プレイヤーをYAMLから読み込み
    if (!mLoader->loadPlayersFromYaml("../assets/data/players.yaml")) {
        std::cerr << "Player YAML load failed" << std::endl;
    }
    // 敵をYAMLから読み込み
    if (!mLoader->loadEnemiesFromYaml("../assets/data/enemies.yaml"))
    {
        std::cerr << "Enemy YAML load failed" << std::endl;
    }
    // ボートをYAMLから読み込み
    if (!mLoader->loadBoatsFromYaml("../assets/data/boats.yaml"))
    {
        std::cerr << "Boats YAML load failed" << std::endl;
    }
    // ボートのかけらをYAMLから読み込み
    if (!mLoader->loadBoatPartsFromYaml("../assets/data/boatParts.yaml"))
    {
        std::cerr << "BoatParts YAML load failed" << std::endl;
    }
    // 鍵をYAMLから読み込み
    if (!mLoader->loadKeysFromYaml("../assets/data/keys.yaml"))
    {
        std::cerr << "keys YAML load failed" << std::endl;
    }

    mMesh = std::make_unique<Mesh>();
    // プレイヤーモデルをロード
    for (auto player : mPlayers) {
        std::string path = "../assets/models/player.obj";
        std::vector<LoadedMesh> playerMeshes = mMesh->loadMeshFromFile(path.c_str());
        player->SetMeshes(playerMeshes);
    }
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
    // 敵モデルをロード
    for (auto planet : planets) {
        std::vector<Enemy*> enemies = planet->GetEnemies();
        for (auto enemy : enemies)
        {
            std::unordered_map<std::string, std::vector<LoadedMesh>> enemyMeshesByPath = mCurrentStage->GetPlanets()[0]->GetEnemyMeshesByPath();
            std::string path = "../assets/models/" + enemy->GetModelPath();
            auto enemyMeshes = mMesh->loadMeshFromFile(path.c_str());
            mCurrentStage->GetPlanets()[0]->AddEnemyMesh(enemy->GetModelPath(), enemyMeshes);
        }
    }
    Planet* currentPlanet = mPlayers[0]->GetCurrentPlanet();
    // 鍵モデルをロード
    Key* key = currentPlanet->GetKey();
    std::vector<LoadedMesh> keyMeshes = mMesh->loadMeshFromFile("../assets/models/key.obj");
    key->SetMeshes(keyMeshes);
    // スターモデルをロード
    Star* star = currentPlanet->GetStar();
    std::vector<LoadedMesh> starMeshes = mMesh->loadMeshFromFile("../assets/models/star.obj");
    star->SetMeshes(starMeshes);
    // ボートモデルをロード
    std::vector<Boat*> boats = currentPlanet->GetBoats();
    std::vector<LoadedMesh> boatMeshes = mMesh->loadMeshFromFile("../assets/models/boat.obj");
    for (auto boat : boats) {
        boat->SetMeshes(boatMeshes);
    }
    std::vector<BoatParts*> boatParts = currentPlanet->GetBoatParts();
    std::vector<LoadedMesh> boatPartsMeshes = mMesh->loadMeshFromFile("../assets/models/boat.obj");
    for (auto parts : boatParts) {
        parts->SetMeshes(boatPartsMeshes);
    }

    // 時間情報
    mLastTime = glfwGetTime();

    // 深度テストをONにして奥行きに応じて描画できるようにする（描画順ではなく、手前にあるものが上書きされて描画される）
    glEnable(GL_DEPTH_TEST);

    // // Bullet Physics：惑星メッシュの当たり判定（惑星ごとに modelPath のメッシュを使用）
    // bool bulletOk = false;
    // btDefaultCollisionConfiguration *bulletCollisionConfig = nullptr;
    // btCollisionDispatcher *bulletDispatcher = nullptr;
    // btBroadphaseInterface *bulletBroadphase = nullptr;
    // btSequentialImpulseConstraintSolver *bulletSolver = nullptr;
    // btDiscreteDynamicsWorld *bulletWorld = nullptr;
    // std::vector<btTriangleMesh *> bulletPlanetMeshes;
    // std::vector<btBvhTriangleMeshShape *> bulletPlanetShapes;
    // std::vector<btRigidBody *> bulletPlanetBodies;
    // btPairCachingGhostObject *bulletGhost = nullptr;
    // btCapsuleShape *bulletCapsule = nullptr;
    // btSphereShape *bulletWallSphere = nullptr; // 壁当たり用スイープ
    // btKinematicCharacterController *bulletCharController = nullptr;

    // if (!planets.empty())
    // {
    //     bulletCollisionConfig = new btDefaultCollisionConfiguration();
    //     bulletDispatcher = new btCollisionDispatcher(bulletCollisionConfig);
    //     bulletBroadphase = new btDbvtBroadphase();
    //     bulletSolver = new btSequentialImpulseConstraintSolver();
    //     bulletWorld = new btDiscreteDynamicsWorld(bulletDispatcher, bulletBroadphase, bulletSolver, bulletCollisionConfig);
    //     bulletWorld->setGravity(btVector3(0, -9.8f, 0));
    //     bulletBroadphase->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());

    //     std::vector<float> pos;
    //     std::vector<unsigned int> idx;
    //     for (size_t p = 0; p < planets.size(); p++)
    //     {
    //         pos.clear();
    //         idx.clear();
    //         std::string meshPath = "../assets/models/" + planets[p]->GetModelPath();
    //         if (!loadMeshPositionsAndIndices(meshPath.c_str(), pos, idx) || pos.size() < 9 || idx.size() < 3)
    //             continue;
    //         const glm::vec3 &center = planets[p]->GetCenter();
    //         float radius = planets[p]->GetRadius();
    //         btTriangleMesh *triMesh = new btTriangleMesh();
    //         for (size_t i = 0; i + 2 < idx.size(); i += 3)
    //         {
    //             unsigned int i0 = idx[i], i1 = idx[i + 1], i2 = idx[i + 2];
    //             if (i0 * 3 + 2 >= pos.size() || i1 * 3 + 2 >= pos.size() || i2 * 3 + 2 >= pos.size())
    //                 continue;
    //             btVector3 v0(center.x + radius * pos[i0 * 3], center.y + radius * pos[i0 * 3 + 1], center.z + radius * pos[i0 * 3 + 2]);
    //             btVector3 v1(center.x + radius * pos[i1 * 3], center.y + radius * pos[i1 * 3 + 1], center.z + radius * pos[i1 * 3 + 2]);
    //             btVector3 v2(center.x + radius * pos[i2 * 3], center.y + radius * pos[i2 * 3 + 1], center.z + radius * pos[i2 * 3 + 2]);
    //             triMesh->addTriangle(v0, v1, v2);
    //         }
    //         bulletPlanetMeshes.emplace_back(triMesh);
    //         // #region agent log
    //         {
    //             std::ostringstream ds;
    //             ds << "{\"planet\":" << p << ",\"numTriangles\":" << triMesh->getNumTriangles() << "}";
    //             debug_log("E", "planet_mesh_built", ds.str());
    //         }
    //         // #endregion
    //         btBvhTriangleMeshShape *shape = new btBvhTriangleMeshShape(triMesh, true);
    //         bulletPlanetShapes.emplace_back(shape);
    //         btTransform startTransform;
    //         startTransform.setIdentity();
    //         startTransform.setOrigin(btVector3(0, 0, 0));
    //         btRigidBody::btRigidBodyConstructionInfo rbInfo(0, nullptr, shape);
    //         btRigidBody *body = new btRigidBody(rbInfo);
    //         body->setWorldTransform(startTransform);
    //         body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
    //         bulletWorld->addRigidBody(body, (short)btBroadphaseProxy::DefaultFilter, (short)-1);
    //         bulletPlanetBodies.emplace_back(body);
    //     }
    //     bulletOk = !bulletPlanetBodies.empty();
    //     // #region agent log
    //     {
    //         debug_log("A", "collision_setup", "{\"ghostGroup\":\"CharacterFilter\",\"ghostMask\":\"-1\",\"staticGroup\":\"DefaultFilter\",\"staticMask\":\"-1\",\"ghostSpawnOffset\":true}");
    //     }
    //     // #endregion

    //     const float capRadius = 0.35f;
    //     const float capHeight = 0.8f;
    //     bulletCapsule = new btCapsuleShape(capRadius, capHeight);
    //     bulletWallSphere = new btSphereShape(0.35f); // 壁スイープ用（キャラ半径程度）
    //     bulletGhost = new btPairCachingGhostObject();
    //     bulletGhost->setCollisionShape(bulletCapsule);
    //     bulletGhost->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
    //     btTransform ghostTrans;
    //     ghostTrans.setIdentity();
    //     glm::vec3 spawnUp = glm::normalize(mPlayers[0]->GetPos() - planets[0]->GetCenter());
    //     float capHalf = capHeight * 0.5f;
    //     glm::vec3 ghostOrigin = mPlayers[0]->GetPos() + spawnUp * (capHalf + 0.15f);
    //     ghostTrans.setOrigin(btVector3(ghostOrigin.x, ghostOrigin.y, ghostOrigin.z));
    //     bulletGhost->setWorldTransform(ghostTrans);
    //     bulletWorld->addCollisionObject(bulletGhost, (short)btBroadphaseProxy::CharacterFilter, (short)-1);
    //     bulletCharController = new btKinematicCharacterController(bulletGhost, bulletCapsule, 0.35f);
    //     bulletCharController->setGravity(btVector3(0, -9.8f, 0));
    //     bulletCharController->setJumpSpeed(5.0f);
    //     bulletCharController->setFallSpeed(55.0f);
    //     bulletWorld->addAction(bulletCharController);
    // }
    // else
    // {
    //     if (!bulletOk)
    //         std::cerr << "Bullet: planet mesh load failed, using sphere collision." << std::endl;
    //     bulletOk = false;
    // }
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
    // ゲーム終了処理
    if (mSdlController)
    {
        SDL_GameControllerClose(mSdlController);
    }
    // for (auto &p : textTextureCache)
    //     glDeleteTextures(1, &p.second.first);
    if (mFont) TTF_CloseFont(mFont);
    TTF_Quit();
    SDL_Quit();

    // 再生中の曲を止める（オーディオは開いたままだから他の曲を流せる）
    Mix_HaltMusic();
    Mix_CloseAudio();

    // if (bulletWorld)
    // {
    //     if (bulletCharController)
    //     {
    //         bulletWorld->removeAction(bulletCharController);
    //         delete bulletCharController;
    //         bulletCharController = nullptr;
    //     }
    //     if (bulletGhost)
    //     {
    //         bulletWorld->removeCollisionObject(bulletGhost);
    //         delete bulletGhost;
    //         bulletGhost = nullptr;
    //     }
    //     if (bulletCapsule)
    //     {
    //         delete bulletCapsule;
    //         bulletCapsule = nullptr;
    //     }
    //     if (bulletWallSphere)
    //     {
    //         delete bulletWallSphere;
    //         bulletWallSphere = nullptr;
    //     }
    //     for (btRigidBody *b : bulletPlanetBodies)
    //     {
    //         if (b)
    //         {
    //             bulletWorld->removeRigidBody(b);
    //             delete b;
    //         }
    //     }
    //     for (btBvhTriangleMeshShape *s : bulletPlanetShapes)
    //     {
    //         if (s)
    //             delete s;
    //     }
    //     for (btTriangleMesh *m : bulletPlanetMeshes)
    //     {
    //         if (m)
    //             delete m;
    //     }
    //     delete bulletWorld;
    //     bulletWorld = nullptr;
    //     delete bulletSolver;
    //     delete bulletBroadphase;
    //     delete bulletDispatcher;
    //     delete bulletCollisionConfig;
    // }

    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

void Game::ProcessInput()
{
    for (const auto& actor_unique : mActors) {
        Actor* actor = actor_unique.get();
        actor->ProcessInput();
    }
    // 敵データのホットリロード
    bool reloadPressed = (glfwGetKey(mWindow, GLFW_KEY_R) == GLFW_PRESS);
    if (reloadPressed && !mReloadKeyPressedPrev)
    {
        if (!mLoader->loadEnemiesFromYaml("../assets/data/enemies.yaml"))
        {
            std::cerr << "Enemy YAML load failed" << std::endl;
        }
        std::vector<Enemy*> enemies = GetCurrentStage()->GetPlanets()[0]->GetEnemies();
        for (auto enemy : enemies)
        {
            std::unordered_map<std::string, std::vector<LoadedMesh>> enemyMeshesByPath = mCurrentStage->GetPlanets()[0]->GetEnemyMeshesByPath();
            if (enemyMeshesByPath.find(enemy->GetModelPath()) == enemyMeshesByPath.end())
            {
                std::string path = "../assets/models/" + enemy->GetModelPath();
                auto enemyMeshes = mMesh->loadMeshFromFile(path.c_str());
                mCurrentStage->GetPlanets()[0]->AddEnemyMesh(enemy->GetModelPath(), enemyMeshes);
            }
        }
    }
    mReloadKeyPressedPrev = reloadPressed;

    // Pキーで2P参加（1回だけ反応）
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

    if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS || (mSdlController && SDL_GameControllerGetButton(mSdlController, SDL_CONTROLLER_BUTTON_BACK)))
        glfwSetWindowShouldClose(mWindow, GLFW_TRUE);
}

void Game::UpdateGame()
{
    double currentTime = glfwGetTime(); 
    float deltaTime = static_cast<float>(currentTime - mLastTime);
    mLastTime = currentTime;

    for (const auto& actor_unique : mActors) {
        Actor* actor = actor_unique.get();
        actor->Update(deltaTime);
    }

    AudioSystem* audioSystem = mAudioSystem.get();
    audioSystem->Update();
}

void Game::GenerateOutput()
{
    glm::mat4 view = mPlayers[0]->getPlayerView();
    glm::mat4 view2P = mIsPlayer2Joined ? mPlayers[1]->getPlayerView() : view;

    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // これから描画するときにどのプログラムを使うのか設定
    glUseProgram(mShader->GetShaderProgram());

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(mWindow, &fbWidth, &fbHeight);

    // 今つながっているコントローラーの数を取得してループ
    for (int i = 0; i < SDL_NumJoysticks(); ++i)
    {
        // i番目がゲームコントローラーとして認識できるか判定
        if (SDL_IsGameController(i))
        {
            // それをゲームコントローラーとして開く
            mSdlController = SDL_GameControllerOpen(i);
        }
    }

    auto drawScene = [&](const glm::mat4 &viewMat, const glm::mat4 &projMat)
    {
        GLint locModel = mShader->GetLocModel();
        GLint locView = mShader->GetLocView();
        GLint locProj = mShader->GetLocProj();
        GLint locObjectColor = mShader->GetLocObjectColor();
        GLint locUseTexture = mShader->GetLocUseTexture();
        GLint locDiffuseTexture = mShader->GetLocDiffuseTexture();

        // CPU側のMVPをシェーダーのそれぞれのuniformに渡して使えるようにしている
        // 惑星描画
        glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(viewMat));
        glUniformMatrix4fv(locProj, 1, GL_FALSE, glm::value_ptr(projMat));

        std::vector<Planet*> planets = mStages[mCurrentStageNum]->GetPlanets();
        auto planetMeshesByPath = mStages[mCurrentStageNum]->GetPlanetMeshesByPath();
        // 惑星描画
        for (size_t i = 0; i < planets.size(); i++)
        {
            glm::mat4 planetModel = glm::translate(glm::mat4(1.0f), planets[i]->GetCenter()) * glm::scale(glm::mat4(1.0f), glm::vec3(planets[i]->GetRadius()));
            glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(planetModel));
            glUniform3f(locObjectColor, planets[i]->GetColor().x, planets[i]->GetColor().y, planets[i]->GetColor().z);
            auto it = planetMeshesByPath.find(planets[i]->GetModelPath());
            if (it != planetMeshesByPath.end() && !it->second.empty())
            {
                for (const LoadedMesh &m : it->second)
                {
                    glBindVertexArray(m.VAO);
                    if (m.textureID != 0)
                    {
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, m.textureID);
                        glUniform1i(locDiffuseTexture, 0);
                        glUniform1i(locUseTexture, 1);
                    }
                    else
                    {
                        glUniform1i(locUseTexture, 0);
                    }
                    glDrawElements(GL_TRIANGLES, m.indexCount, GL_UNSIGNED_INT, 0);
                }
                glUniform1i(locUseTexture, 0);
            }
            else
            {
                // glBindVertexArray(sphereVAO);
                // glUniform1i(locUseTexture, 0);
                // glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
            }
        }

        auto drawCharacter = [&](const glm::vec3 &pos, float scale, const glm::vec3 &fallbackColor,
                                 const glm::vec3 &up, float yaw, const std::vector<LoadedMesh> &meshes,
                                 const glm::vec3 *colorOverride = nullptr)
        {
            glm::vec3 upN = glm::normalize(up);
            glm::vec3 worldLeft = glm::normalize(glm::cross(upN, glm::vec3(0, 0, 1)));
            if (glm::length(worldLeft) < 0.01f)
                worldLeft = glm::normalize(glm::cross(upN, glm::vec3(1, 0, 0)));
            glm::vec3 fwd = glm::normalize(glm::cross(worldLeft, upN) * std::cos(yaw) - std::sin(yaw) * worldLeft);
            glm::vec3 left = glm::normalize(glm::cross(upN, fwd));
            glm::vec3 right = -left;
            glm::mat4 orient = glm::mat4(1.0f);
            orient[0] = glm::vec4(-fwd, 0.0f);
            orient[1] = glm::vec4(up, 0.0f);
            orient[2] = glm::vec4(right, 0.0f);
            orient[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            glm::mat4 model = glm::translate(glm::mat4(1.0f), pos) * orient * glm::scale(glm::mat4(1.0f), glm::vec3(scale));
            if (!meshes.empty())
            {
                glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
                for (const LoadedMesh &m : meshes)
                {
                    glBindVertexArray(m.VAO);
                    if (m.textureID != 0)
                    {
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, m.textureID);
                        glUniform1i(locDiffuseTexture, 0);
                        glUniform1i(locUseTexture, 1);
                    }
                    else
                    {
                        glUniform1i(locUseTexture, 0);
                    }
                    if (colorOverride)
                    {
                        glUniform3f(locObjectColor, colorOverride->x, colorOverride->y, colorOverride->z);
                    }
                    else
                    {
                        glUniform3f(locObjectColor, m.diffuseColor[0], m.diffuseColor[1], m.diffuseColor[2]);
                    }
                    glDrawElements(GL_TRIANGLES, m.indexCount, GL_UNSIGNED_INT, 0);
                }
                glUniform1i(locUseTexture, 0);
            }
            else
            {
                // glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
                // glBindVertexArray(VAO);
                // glm::vec3 c = colorOverride ? *colorOverride : fallbackColor;
                // glUniform3f(locObjectColor, c.x, c.y, c.z);
                // glDrawArrays(GL_TRIANGLES, 0, 3);
            }
        };

        const float playerScale = 0.25f;
        // 1Pの描画
        glm::vec3 up0 = glm::normalize(mPlayers[0]->GetPos() - planets[mPlayers[0]->GetCurrentPlanetNum()]->GetCenter());
        drawCharacter(mPlayers[0]->GetPos(), playerScale, glm::vec3(0.0f, 0.0f, 1.0f), up0, mPlayers[0]->GetFacingYaw(), mPlayers[0]->GetMeshes());

        // 2Pの描画
        if (mIsPlayer2Joined)
        {
            glm::vec3 up1 = glm::normalize(mPlayers[1]->GetPos() - planets[mPlayers[1]->GetCurrentPlanetNum()]->GetCenter());
            drawCharacter(mPlayers[1]->GetPos(), playerScale, glm::vec3(1.0f, 0.5f, 0.0f), up1, mPlayers[1]->GetFacingYaw(),  mPlayers[1]->GetMeshes());
        }

        std::vector<Enemy*> enemies = mCurrentStage->GetPlanets()[mPlayers[0]->GetCurrentPlanetNum()]->GetEnemies();
        // 敵描画
        for (size_t ei = 0; ei < enemies.size(); ei++)
        {
            Enemy*& enemy = enemies[ei];
            if (!enemy->GetIsAlive())
                continue;
            std::unordered_map<std::string, std::vector<LoadedMesh>> enemyMeshesByPath = mCurrentStage->GetPlanets()[0]->GetEnemyMeshesByPath();
            auto eit = enemyMeshesByPath.find(enemy->GetModelPath());
            if (eit == enemyMeshesByPath.end() || eit->second.empty())
                eit = enemyMeshesByPath.find("enemy.obj");
            if (eit == enemyMeshesByPath.end() || eit->second.empty())
                continue;
            glm::vec3 enemyUp = glm::normalize(enemy->GetPos() - enemy->GetCurrentPlanet()->GetCenter());
            glm::vec3 toPlayer = glm::normalize(mPlayers[0]->GetPos() - enemy->GetPos());
            float enemyFacingYaw = mPlayers[0]->getYawFromDirection(enemyUp, toPlayer) + 3.14159265f;
            drawCharacter(enemy->GetPos(), enemy->GetScale(), glm::vec3(0.0f, 1.0f, 0.0f), enemyUp, enemyFacingYaw, eit->second);
            // 敵の頭上にID（1始まり）をビルボード表示
            if (mFont)
            {
                std::unordered_map<std::string, std::pair<GLuint, glm::ivec2>> textTextureCache;
                // 文字列→テクスチャ（敵ID表示用、キャッシュ付き）
                auto getTextTexture = [&](const std::string &s) -> std::pair<GLuint, glm::ivec2>
                {
                    if (!mFont || s.empty())
                        return {0, {0, 0}};
                    auto it = textTextureCache.find(s);
                    if (it != textTextureCache.end())
                        return it->second;
                    SDL_Color white = {255, 255, 255, 255};
                    SDL_Surface *surf = TTF_RenderText_Blended(mFont, s.c_str(), white);
                    if (!surf)
                        return {0, {0, 0}};
                    SDL_Surface *rgba = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
                    SDL_FreeSurface(surf);
                    if (!rgba)
                        return {0, {0, 0}};
                    int tw = rgba->w, th = rgba->h;
                    GLuint tex;
                    glGenTextures(1, &tex);
                    glBindTexture(GL_TEXTURE_2D, tex);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba->pixels);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    SDL_FreeSurface(rgba);
                    textTextureCache[s] = {tex, {tw, th}};
                    return {tex, {tw, th}};
                };
                auto [texId, texSize] = getTextTexture(std::to_string(ei + 1));
                if (texId != 0 && texSize.x > 0 && texSize.y > 0)
                {
                    glm::vec3 camPos(glm::inverse(viewMat)[3]);
                    glm::vec3 quadCenter = enemy->GetPos() + enemyUp * 0.8f;
                    glm::vec3 forward = glm::normalize(camPos - quadCenter);
                    glm::vec3 right = glm::normalize(glm::cross(enemyUp, forward));
                    if (glm::length(right) < 0.01f)
                        right = glm::normalize(glm::cross(enemyUp, glm::vec3(0, 0, 1)));
                    glm::vec3 upQuad = glm::cross(forward, right);
                    // 敵のどれくらい上にラベルを描画するのか
                    const float enemyLabelHeight = 0.5f;
                    float w = enemyLabelHeight * static_cast<float>(texSize.x) / static_cast<float>(texSize.y);
                    glm::mat4 billboard(1.0f);
                    billboard[0] = glm::vec4(right * w, 0.0f);
                    billboard[1] = glm::vec4(upQuad * enemyLabelHeight, 0.0f);
                    billboard[2] = glm::vec4(forward, 0.0f);
                    billboard[3] = glm::vec4(quadCenter, 1.0f);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glDepthMask(GL_FALSE);
                    glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(billboard));
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, texId);
                    glUniform1i(locUseTexture, 1);
                    glUniform3f(locObjectColor, 1.0f, 1.0f, 1.0f);
                    mVertexArrays["text"]->SetActive();
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                    glUniform1i(locUseTexture, 0);
                    glDepthMask(GL_TRUE);
                    glDisable(GL_BLEND);
                }
            }
        }

        // 鍵描画
        Planet* currentPlanet = mPlayers[0]->GetCurrentPlanet();
        Key* key = currentPlanet->GetKey();
        if (key->GetCollectableComponent()->GetIsActive())
        {
            const float keyScale = 2.0f;
            const glm::vec3 keyColor(0.85f, 0.65f, 0.13f); // 金色
            glm::vec3 keyUp = glm::normalize(key->GetPos() - currentPlanet->GetCenter());
            drawCharacter(key->GetPos(), keyScale, keyColor, keyUp, 0.0f, key->GetMeshes(), &keyColor);
        }

        std::vector<Boat*> boats = currentPlanet->GetBoats();
        // ボート描画
        for (auto boat : boats) {
            if (boat->GetIsActive())
            {
                const float boatScale = 0.8f;
                glm::vec3 boatUp = glm::normalize(boat->GetPos() - currentPlanet->GetCenter());
                drawCharacter(boat->GetPos(), boatScale, glm::vec3(0.4f, 0.25f, 0.1f), boatUp, 0.0f, boat->GetMeshes());
            }
        }

        // ボートのかけら描画
        std::vector<BoatParts*> boatParts = currentPlanet->GetBoatParts();
        for (auto parts : boatParts) { 
            if (parts->GetCollectableComponent()->GetIsActive()) {
                const float boatPartsScale = 1.0f;
                glm::vec3 boatPartsUp = glm::normalize(parts->GetPos() - currentPlanet->GetCenter());
                drawCharacter(parts->GetPos(), boatPartsScale, glm::vec3(0.4f, 0.25f, 0.1f), boatPartsUp, 0.0f, parts->GetMeshes());
            }
        }

        // スター描画
        Star* star = currentPlanet->GetStar();
        if (star->GetIsActive())
        {
            glm::vec3 starUp = glm::normalize(star->GetPos() - currentPlanet->GetCenter());
            glm::vec3 starColor(1.0f, 0.9f, 0.2f);
            const float starScale = 0.3f;
            drawCharacter(star->GetPos(), starScale, starColor, starUp, 0.0f, star->GetMeshes());
        }

        mUIRenderer->DrawTextBox(fbWidth, fbHeight, 40, 400, 200, 0, 0, {0.0f, 0.0f, 0.0f}, "こんにちは");
    };

    if (!mIsPlayer2Joined)
    {
        glViewport(0, 0, fbWidth, fbHeight);
        float aspect = static_cast<float>(fbWidth) / static_cast<float>(fbHeight);
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
        drawScene(view, proj);
    }
    else
    {
        float halfW = fbWidth * 0.5f;
        float aspectHalf = halfW / static_cast<float>(fbHeight);
        glm::mat4 projHalf = glm::perspective(glm::radians(45.0f), aspectHalf, 0.1f, 100.0f);
        glViewport(0, 0, static_cast<GLsizei>(halfW), fbHeight);
        drawScene(view, projHalf);
        glViewport(static_cast<GLsizei>(halfW), 0, static_cast<GLsizei>(halfW), fbHeight);
        drawScene(view2P, projHalf);
    }

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