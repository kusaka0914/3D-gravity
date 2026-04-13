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

    mShader = std::make_unique<Shader>();
    if (!mShader->GetShaderProgram())
    {
        glfwTerminate();
        return false;
    }

    // コントローラー設定
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
    auto audioSystem = std::make_unique<AudioSystem>(this);
    mAudioSystem = std::move(audioSystem);

    // ステージ作成
    auto stageUnique = std::make_unique<Stage>(this);
    Stage* stage = stageUnique.get();
    mActors.emplace_back(std::move(stageUnique));
    mStages.emplace_back(stage);
    mCurrentStage = mStages[0];

    // プレイヤー作成
    auto playerUnique = std::make_unique<Player>(this);
    Player* player = playerUnique.get();
    mActors.emplace_back(std::move(playerUnique));
    mPlayers.emplace_back(player);

    Planet* planet = mStages[0]->GetPlanets()[0];

    // 描画する三角形の3頂点の座標（プレイヤー用フォールバック）
    std::vector<float> fallbackTriangleVertices;
    player->getPlayerFallbackTriangle(fallbackTriangleVertices);
    mVertexArrays["triangle"] = std::make_unique<VertexArray>(fallbackTriangleVertices.data(), 9, nullptr, 0);

    // 球の頂点座標を格納する配列
    std::vector<float> sphereVertices;
    // どの頂点で三角形を作るかを表すインデックスの配列
    std::vector<unsigned int> sphereIndices;
    planet->buildSphereMesh(18, 18, 1.0f, sphereVertices, sphereIndices);
    mVertexArrays["sphere"] = std::make_unique<VertexArray>(sphereVertices.data(), 972, sphereIndices.data(), 1944);

    auto mesh = std::make_unique<Mesh>();

    for (auto player : mPlayers) {
        std::string path = "../assets/models/player.obj";
        std::vector<LoadedMesh> playerMeshes = mesh->loadMeshFromFile(path.c_str());
        player->SetPlayerMeshes(playerMeshes);
    }
    // 鍵モデルをロードする（敵全員撃破で出現）
    std::vector<LoadedMesh> keyMeshes = mesh->loadMeshFromFile("../assets/models/key.obj");
    // ボートモデル（鍵取得で惑星近くに出現）
    std::vector<LoadedMesh> boatMeshes = mesh->loadMeshFromFile("../assets/models/boat.obj");
    // スター（惑星2に配置、触れるとゲームクリア）
    std::vector<LoadedMesh> starMeshes = mesh->loadMeshFromFile("../assets/models/star.obj");

    // フォント
    mFont = nullptr;
    const char *fontPaths[] = {
        "../assets/fonts/font.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/Library/Fonts/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    };
    // 最初に見つかったフォントを用いる
    for (const char *path : fontPaths)
    {
        mFont = TTF_OpenFont(path, 24);
        if (mFont)
            break;
    }
    // フォント失敗処理
    if (!mFont)
        std::cerr << "No mFont loaded. Enemy ID labels will not be shown." << std::endl;

    std::unordered_map<std::string, std::pair<GLuint, glm::ivec2>> textTextureCache;
    // 敵のどれくらい上にラベルを描画するのか
    const float enemyLabelHeight = 0.5f;

    // テキスト用四角形
    unsigned int textQuadVAO, textQuadVBO;
    {
        float quad[] = {
            -0.5f,
            -0.5f,
            0.0f,
            0.0f,
            0.0f,
            1.0f,
            0.0f,
            1.0f,
            0.5f,
            -0.5f,
            0.0f,
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            1.0f,
            0.5f,
            0.5f,
            0.0f,
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            0.0f,
            -0.5f,
            -0.5f,
            0.0f,
            0.0f,
            0.0f,
            1.0f,
            0.0f,
            1.0f,
            0.5f,
            0.5f,
            0.0f,
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            0.0f,
            -0.5f,
            0.5f,
            0.0f,
            0.0f,
            0.0f,
            1.0f,
            0.0f,
            0.0f,
        };
        glGenVertexArrays(1, &textQuadVAO);
        glGenBuffers(1, &textQuadVBO);
        glBindVertexArray(textQuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, textQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }

    // 惑星をYAMLから読み込み
    auto loader = std::make_unique<Loader>(this);
    static const char *const planetsYamlPath = "../assets/data/planets.yaml";
    if (!loader->loadPlanetsFromYaml(planetsYamlPath)) {
        std::cerr << "Planet YAML load failed" << std::endl;
    }

    std::vector<Planet*> planets = mCurrentStage->GetPlanets();
    // 惑星ごとのモデルをロード
    for (auto planet : planets)
    {
        std::unordered_map<std::string, std::vector<LoadedMesh>> planetMeshesByPath = mCurrentStage->GetPlanetMeshesByPath();
        if (planetMeshesByPath.find(planet->GetModelPath()) == planetMeshesByPath.end())
        {
            std::string path = "../assets/models/" + planet->GetModelPath();
            auto planetMeshes = mesh->loadMeshFromFile(path.c_str());
            mCurrentStage->AddPlanetMesh(planet->GetModelPath(), planetMeshes);
        }
    }

    // // 1P/2P 状態（players[0]=1P, mPlayers[1]=2P）
    // PlayerState mPlayers[2];
    // static const char *const playersYamlPath = "../assets/data/mPlayers.yaml";
    // PlayerInitialConfig playerInitial;
    // if (loadPlayerFromYaml(playersYamlPath, planets, playerInitial))
    // {
    //     mPlayers[0]->GetPos() = playerInitial->GetPos();
    //     mPlayers[0].planetIndex = playerInitial.planetIndex;
    //     mPlayers[0].cameraPitch = playerInitial.cameraPitch;
    //     mPlayers[0].attack = playerInitial.attack;
    //     mPlayers[0].hp = playerInitial.hp;
    // }
    // else
    // {
    //     mPlayers[0]->GetPos() = glm::vec3(8.0f, planets[0]->GetRadius(), 2.5f);
    //     mPlayers[0].planetIndex = 0;
    //     mPlayers[0].cameraPitch = 0.4f;
    //     mPlayers[0].attack = 10.0f;
    //     mPlayers[0].hp = 100.0f;
    //     std::cerr << "Player YAML load failed, using default 1P config." << std::endl;
    // }
    // glm::vec3 restartPos = mPlayers[0]->GetPos();
    // int restartPlanetIndex = 0;
    // bool mIsPlayer2Joined = false;

    static const char *const enemiesYamlPath = "../assets/data/enemies.yaml";
    // 敵を YAML から読み込み（失敗時は1体のデフォルト）
    if (!loader->loadEnemiesFromYaml(enemiesYamlPath))
    {
        std::cerr << "Enemy YAML load failed, using 1 default enemy." << std::endl;
    }
    // 敵ごとのモデルをロード
    std::vector<Enemy*> enemies = GetCurrentStage()->GetPlanets()[0]->GetEnemies();
    for (auto enemy : enemies)
    {
        std::unordered_map<std::string, std::vector<LoadedMesh>> enemyMeshesByPath = mCurrentStage->GetPlanets()[0]->GetEnemyMeshesByPath();
        if (enemyMeshesByPath.find(enemy->GetModelPath()) == enemyMeshesByPath.end())
        {
            std::string path = "../assets/models/" + enemy->GetModelPath();
            auto enemyMeshes = mesh->loadMeshFromFile(path.c_str());
            mCurrentStage->GetPlanets()[0]->AddEnemyMesh(enemy->GetModelPath(), enemyMeshes);
        }
    }

    // // 鍵（現在の惑星で最後に倒した敵の場所に出現）
    // bool keyVisible = false;
    // bool keyObtained = false;
    // glm::vec3 keyPos(0.0f);
    // int keyPlanetIndex = -1;
    // glm::vec3 lastDefeatedEnemyPos(0.0f); // 現在惑星で最後に倒した敵の位置
    // const float keyScale = 2.0f;
    // // const float keyPickupRadius = 1.2f;
    // const glm::vec3 keyColor(0.85f, 0.65f, 0.13f); // 金色

    // // ボート
    // glm::vec3 boatPos(0.0f);
    // const float boatScale = 0.8f;
    // const float boatTouchRadius = 1.8f;

    // // ボートで次の惑星へ移動（触れたら少しずつ移動して到着）
    // bool boatTransitionActive = false;
    // float boatTransitionTimer = 0.0f;
    // const float boatTransitionDuration = 5.0f;
    // glm::vec3 boatTransitionStartBoat, boatTransitionEnd;
    // int boatDestinationPlanetIndex = -1;
    // int currentBgmPlanetIndex = -1; // 現在流しているBGMの惑星（惑星2ならboss.wav）

    // // スター（鍵ルートの到着先 or ボス撃破で出現）
    // glm::vec3 starPos(0.0f);
    // const float starScale = 0.3f;
    // const float starTouchRadius = 1.5f;
    // bool gameClear = false;
    // bool starVisibleFromBoss = false; // ボス撃破でスター出現したか
    // int starBossPlanetIndex = -1;     // ボス撃破で出たスターの惑星番号

    // // 時間情報
    mLastTime = glfwGetTime();

    // // 深度テストをONにして奥行きに応じて描画できるようにする（描画順ではなく、手前にあるものが上書きされて描画される）
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

    // // 文字列→テクスチャ（敵ID表示用、キャッシュ付き）
    // auto getTextTexture = [&](const std::string &s) -> std::pair<GLuint, glm::ivec2>
    // {
    //     if (!mFont || s.empty())
    //         return {0, {0, 0}};
    //     auto it = textTextureCache.find(s);
    //     if (it != textTextureCache.end())
    //         return it->second;
    //     SDL_Color white = {255, 255, 255, 255};
    //     SDL_Surface *surf = TTF_RenderText_Blended(mFont, s.c_str(), white);
    //     if (!surf)
    //         return {0, {0, 0}};
    //     SDL_Surface *rgba = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
    //     SDL_FreeSurface(surf);
    //     if (!rgba)
    //         return {0, {0, 0}};
    //     int tw = rgba->w, th = rgba->h;
    //     GLuint tex;
    //     glGenTextures(1, &tex);
    //     glBindTexture(GL_TEXTURE_2D, tex);
    //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba->pixels);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //     SDL_FreeSurface(rgba);
    //     textTextureCache[s] = {tex, {tw, th}};
    //     return {tex, {tw, th}};
    // };
    return true;
}

void Game::RunLoop()
{
    // ゲームループ
    while (!glfwWindowShouldClose(mWindow))
    {
        // 入力などのイベントを処理する
        glfwPollEvents();
        ProcessInput();
        UpdateGame();
        GenerateOutput();
    }
}

void Game::Shutdown()
{
    // // ゲーム終了処理
    // if (mSdlController)
    // {
    //     SDL_GameControllerClose(mSdlController);
    // }
    // for (auto &p : textTextureCache)
    //     glDeleteTextures(1, &p.second.first);
    // if (mFont) TTF_CloseFontmFontt);
    // TTF_Quit();
    // glDeleteVertexArrays(1, &textQuadVAO);
    // glDeleteBuffers(1, &textQuadVBO);
    // SDL_Quit();

    // // 再生中の曲を止める（オーディオは開いたままだから他の曲を流せる）
    // Mix_HaltMusic();
    // // 曲データを解放する
    // if (normalBGM)
    //     Mix_FreeMusic(normalBGM);
    // if (bossBGM)
    //     Mix_FreeMusic(bossBGM);
    // if (attackSE)
    //     Mix_FreeChunk(attackSE);
    // if (attackMissSE)
    //     Mix_FreeChunk(attackMissSE);
    // if (attackPreSE)
    //     Mix_FreeChunk(attackPreSE);
    // if (counterSE)
    //     Mix_FreeChunk(counterSE);
    // if (clearSE)
    //     Mix_FreeChunk(clearSE);
    // // オーディオを閉じる（曲を流せなくなる）
    // Mix_CloseAudio();

    // glDeleteVertexArrays(1, &VAO);
    // glDeleteBuffers(1, &VBO);
    // glDeleteProgram(shaderProgram);

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

    // glfwDestroyWindow(mWindow);
    // glfwTerminate();
}

void Game::ProcessInput()
{
    for (const auto& actor_unique : mActors) {
        Actor* actor = actor_unique.get();
        actor->ProcessInput();
    }
    // // 敵データのホットリロード
    // bool reloadPressed = (glfwGetKey(mWindow, GLFW_KEY_R) == GLFW_PRESS);
    // if (reloadPressed && !mReloadKeyPressedPrev)
    // {
    //     std::vector<std::unique_ptr<Enemy>> newEnemies;
    //     if (loadEnemiesFromYaml(enemiesYamlPath, planets, newEnemies))
    //     {
    //         enemies = std::move(newEnemies);
    //         enemyMeshesByPath.clear();
    //         for (const unique_ptr<Enemy> &p : enemies)
    //         {
    //             const std::string &pathKey = p->modelPath;
    //             if (enemyMeshesByPath.find(pathKey) == enemyMeshesByPath.end())
    //             {
    //                 std::string path = "../assets/models/" + pathKey;
    //                 enemyMeshesByPath[pathKey] = loadMeshFromFile(path.c_str());
    //             }
    //         }
    //         std::cout << "Enemies reloaded (" << enemies.size() << " enemies)" << std::endl;
    //     }
    //     else
    //     {
    //         std::cout << "Enemy reload failed." << std::endl;
    //     }
    // }
    // mReloadKeyPressedPrev = reloadPressed;

    // // Pキーで2P参加（1回だけ反応）
    // bool pKeyNow = (glfwGetKey(mWindow, GLFW_KEY_P) == GLFW_PRESS);
    // if (pKeyNow && !mIsPlayer2Joined)
    // {
    //     mIsPlayer2Joined = true;
    //     mPlayers[1] = mPlayers[0];
    //     mPlayers[1]->GetPos() = planets[mPlayers[0].planetIndex]->GetCenter() + glm::normalize(mPlayers[0]->GetPos() - planets[mPlayers[0].planetIndex]->GetCenter()) * planets[mPlayers[0].planetIndex]->GetRadius();
    //     glm::vec3 fwdP, ignoreL;
    //     getForwardLeft(up, mPlayers[0].cameraYaw, fwdP, ignoreL);
    //     mPlayers[1]->GetPos() += fwdP * 2.0f;
    //     mPlayers[1]->GetPos() = planets[mPlayers[0].planetIndex]->GetCenter() + glm::normalize(mPlayers[1]->GetPos() - planets[mPlayers[0].planetIndex]->GetCenter()) * planets[mPlayers[0].planetIndex]->GetRadius();
    //     mPlayers[1].velocity = glm::vec3(0, 0, 0);
    //     mPlayers[1].onGround = true;
    //     mPlayers[1].attack = 10.0f;
    //     mPlayers[1].hp = 100.0f;
    // } 

    // if (glfwGetKey(mWindow, GLFW_KEY_L) == GLFW_PRESS)
    //     counterPressed = true;

    // if (mIsPlayer2Joined)
    // {
    //     float moveF2 = 0.0f, moveL2 = 0.0f;
    //     bool jump2 = (glfwGetKey(mWindow, GLFW_KEY_SPACE) == GLFW_PRESS);
    //     float dash2 = (glfwGetKey(mWindow, GLFW_KEY_R) == GLFW_PRESS) ? 1.5f : 1.0f;
    //     if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS)
    //         moveF2 -= 1.0f;
    //     if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS)
    //         moveF2 += 1.0f;
    //     if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS)
    //         moveL2 -= 1.0f;
    //     if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS)
    //         moveL2 += 1.0f;
    //     if (glfwGetKey(mWindow, GLFW_KEY_Q) == GLFW_PRESS)
    //         mPlayers[1].cameraYaw -= cameraSensitivity * deltaTime;
    //     if (glfwGetKey(mWindow, GLFW_KEY_E) == GLFW_PRESS)
    //         mPlayers[1].cameraYaw += cameraSensitivity * deltaTime;

    //     glm::vec3 up2 = glm::normalize(mPlayers[1]->GetPos() - planets[mPlayers[1].planetIndex]->GetCenter());
    //     glm::vec3 fwd2, left2;
    //     getForwardLeft(up2, mPlayers[1].cameraYaw, fwd2, left2);
    //     if (std::abs(moveF2) > 0.01f || std::abs(moveL2) > 0.01f)
    //     {
    //         glm::vec3 moveDir2 = fwd2 * moveF2 + left2 * moveL2;
    //         float len2 = glm::length(moveDir2);
    //         if (len2 > 0.001f)
    //         {
    //             moveDir2 /= len2;
    //             mPlayers[1]->GetFacingYaw() = getYawFromDirection(up2, moveDir2) + 3.14159265f;
    //         }
    //     }
    //     if (transitionTimer <= 0.0f)
    //     {
    //         mPlayers[1]->GetPos() += fwd2 * moveF2 * characterSpeed * deltaTime * dash2;
    //         mPlayers[1]->GetPos() += left2 * moveL2 * characterSpeed * deltaTime * dash2;
    //         if (mPlayers[1].onGround && jump2)
    //         {
    //             mPlayers[1].velocity += up2 * 5.0f;
    //             mPlayers[1].onGround = false;
    //         }
    //     }
    //     // updatePlayerPhysics(mPlayers[1], deltaTime, planets, &transitionTimer);
    // }

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

    // ボート移動
    // if (boatTransitionActive)
    // {
    //     boatTransitionTimer += deltaTime;
    //     // ボード移動がどれくらい進んだかの割合
    //     float t = glm::min(1.0f, boatTransitionTimer / boatTransitionDuration);
    //     // smoothstep で滑らかに
    //     t = t * t * (3.0f - 2.0f * t);
    //     boatPos = boatTransitionStartBoat + (boatTransitionEnd - boatTransitionStartBoat) * t;
    //     // ボート位置から一番近い惑星の「上」方向でプレイヤーをボートの上に
    //     int nearestIdx = 0;
    //     float nearestD = glm::length(boatPos - planets[0]->GetCenter());
    //     for (size_t i = 1; i < planets.size(); i++)
    //     {
    //         float d = glm::length(boatPos - planets[i]->GetCenter());
    //         if (d < nearestD)
    //         {
    //             nearestD = d;
    //             nearestIdx = static_cast<int>(i);
    //         }
    //     }
    //     glm::vec3 boatUp = glm::normalize(boatPos - planets[nearestIdx]->GetCenter());
    //     mPlayers[0]->GetPos() = boatPos + boatUp * playerHeightAboveBoat;
    //     // 到着処理
    //     if (t >= 1.0f && boatDestinationPlanetIndex >= 0)
    //     {
    //         mPlayers[0].planetIndex = boatDestinationPlanetIndex;
    //         boatPos = boatTransitionEnd;
    //         mPlayers[0]->GetPos() = boatTransitionEnd;
    //         mPlayers[0].onGround = true;
    //         mPlayers[0].velocity = glm::vec3(0.0f);
    //         restartPos = boatTransitionEnd;
    //         restartPlanetIndex = boatDestinationPlanetIndex;
    //         boatTransitionActive = false;
    //         if (bulletGhost)
    //         {
    //             btTransform t;
    //             t.setIdentity();
    //             t.setOrigin(btVector3(mPlayers[0]->GetPos().x, mPlayers[0]->GetPos().y, mPlayers[0]->GetPos().z));
    //             bulletGhost->setWorldTransform(t);
    //         }
    //     }
    // }
    // if (!boatTransitionActive)
    // {        
    //     up = glm::normalize(mPlayers[0]->GetPos() - planets[mPlayers[0].planetIndex]->GetCenter());
    // }

    // 惑星2にいるときはBGMをboss.wavに切り替え（ゲームクリア後はBGMを流さない）
    // if (!gameClear && mPlayers[0].planetIndex != currentBgmPlanetIndex)
    // {
    //     currentBgmPlanetIndex = mPlayers[0].planetIndex;
    //     if (currentBgmPlanetIndex == 1 && bossBGM)
    //     {
    //         Mix_PlayMusic(bossBGM, -1);
    //     }
    //     else if (normalBGM)
    //     {
    //         Mix_PlayMusic(normalBGM, -1);
    //     }
    // }

    // glm::vec3 forward, left;
    // getForwardLeft(up, mPlayers[0].cameraYaw, forward, left);

    // // スティックを倒した方向を向く。移動ロック中は地上のみ向き固定、空中攻撃中は向き替え可
    // if ((mPlayers[0].attackMoveLockRemaining <= 0.0f || !mPlayers[0].onGround) && (std::abs(moveForward) > 0.01f || std::abs(moveLeft) > 0.01f))
    // {
    //     glm::vec3 moveDir = forward * moveForward + left * moveLeft;
    //     float len = glm::length(moveDir);
    //     if (len > 0.001f)
    //     {
    //         // 移動方向を正規化
    //         moveDir /= len;
    //         mPlayers[0]->GetFacingYaw() = getYawFromDirection(up, moveDir) + 3.14159265f;
    //     }
    // }

    // if (!boatTransitionActive)
    // {
    //     if (transitionTimer <= 0.0f && !mPlayers[0].isDamaged && mPlayers[0].attackMoveLockRemaining <= 0.0f && dodgeTimer <= 0.0f)
    //     {
    //         glm::vec3 moveDelta = forward * moveForward * characterSpeed * deltaTime * dashSpeed + left * moveLeft * characterSpeed * deltaTime * dashSpeed;
    //         glm::vec3 desiredPos = mPlayers[0]->GetPos() + moveDelta;
    //         // 壁当たり：球スイープで移動経路に障害があれば移動を打ち切り
    //         if (bulletOk && bulletWorld && bulletWallSphere && glm::length(moveDelta) > 1e-5f)
    //         {
    //             glm::vec3 upForSweep = glm::normalize(mPlayers[0]->GetPos() - planets[mPlayers[0].planetIndex]->GetCenter());
    //             glm::vec3 sweepFrom = mPlayers[0]->GetPos() + upForSweep * 0.4f; // 腰高で判定（地面に当たりにくくする）
    //             glm::vec3 sweepTo = desiredPos + upForSweep * 0.4f;
    //             btTransform fromBt, toBt;
    //             fromBt.setIdentity();
    //             fromBt.setOrigin(btVector3(sweepFrom.x, sweepFrom.y, sweepFrom.z));
    //             toBt.setIdentity();
    //             toBt.setOrigin(btVector3(sweepTo.x, sweepTo.y, sweepTo.z));
    //             btVector3 sweepFromBt(sweepFrom.x, sweepFrom.y, sweepFrom.z);
    //             btVector3 sweepToBt(sweepTo.x, sweepTo.y, sweepTo.z);
    //             btCollisionWorld::ClosestConvexResultCallback sweepCallback(sweepFromBt, sweepToBt);
    //             bulletWorld->convexSweepTest(bulletWallSphere, fromBt, toBt, sweepCallback);
    //             if (sweepCallback.hasHit())
    //             {
    //                 // 壁手前で一度止め、残りを壁に沿う方向（スライド）に投影して進める
    //                 float allowFrac = std::max(0.0f, sweepCallback.m_closestHitFraction - 0.02f);
    //                 glm::vec3 posAfterHit = mPlayers[0]->GetPos() + moveDelta * allowFrac;
    //                 glm::vec3 hitNormGlm(
    //                     sweepCallback.m_hitNormalWorld.x(),
    //                     sweepCallback.m_hitNormalWorld.y(),
    //                     sweepCallback.m_hitNormalWorld.z());
    //                 // 阻害された移動を壁面に投影 → 壁沿いのスライドベクトル
    //                 glm::vec3 blocked = moveDelta * (1.0f - allowFrac);
    //                 glm::vec3 slideVec = blocked - hitNormGlm * glm::dot(blocked, hitNormGlm);
    //                 const float slideEps = 1e-4f;
    //                 if (glm::length(slideVec) > slideEps)
    //                 {
    //                     glm::vec3 slideFrom = posAfterHit + upForSweep * 0.4f;
    //                     glm::vec3 slideTo = slideFrom + slideVec;
    //                     btTransform fromBt2, toBt2;
    //                     fromBt2.setIdentity();
    //                     fromBt2.setOrigin(btVector3(slideFrom.x, slideFrom.y, slideFrom.z));
    //                     toBt2.setIdentity();
    //                     toBt2.setOrigin(btVector3(slideTo.x, slideTo.y, slideTo.z));
    //                     btVector3 sFrom(slideFrom.x, slideFrom.y, slideFrom.z);
    //                     btVector3 sTo(slideTo.x, slideTo.y, slideTo.z);
    //                     btCollisionWorld::ClosestConvexResultCallback slideCallback(sFrom, sTo);
    //                     bulletWorld->convexSweepTest(bulletWallSphere, fromBt2, toBt2, slideCallback);
    //                     float slideAllow = slideCallback.hasHit()
    //                                            ? std::max(0.0f, slideCallback.m_closestHitFraction - 0.02f)
    //                                            : 1.0f;
    //                     desiredPos = posAfterHit + slideVec * slideAllow;
    //                 }
    //                 else
    //                 {
    //                     desiredPos = posAfterHit;
    //                 }
    //             }
    //         }
    //         mPlayers[0]->GetPos() = desiredPos;
    //         if (mPlayers[0].onGround && jumpPressed)
    //         {
    //             mPlayers[0].velocity += up * 5.0f;
    //             mPlayers[0].onGround = false;
    //         }
    //     }
    //     // Bボタン：向いている方向へ回避開始
    //     if (dodgePressed && !dodgePressedPrev && dodgeCooldown <= 0.0f && dodgeTimer <= 0.0f && mPlayers[0].attackDodgeLockRemaining <= 0.0f)
    //     {
    //         glm::vec3 dodgeFwd, dodgeLeftUnused;
    //         getForwardLeft(up, mPlayers[0]->GetFacingYaw(), dodgeFwd, dodgeLeftUnused);
    //         dodgeDir = -dodgeFwd;
    //         dodgeTimer = dodgeDuration;
    //         dodgeCooldown = dodgeCooldownTime;
    //         dodgeStartHeight = glm::length(mPlayers[0]->GetPos() - planets[mPlayers[0].planetIndex]->GetCenter());
    //         mPlayers[0].velocity = glm::vec3(0.0f); // 回避後に通常落下するためリセット
    //     }
    //     if (dodgeTimer > 0.0f)
    //     {
    //         float dodgeSpeed = dodgeDistance / dodgeDuration;
    //         mPlayers[0]->GetPos() += dodgeDir * dodgeSpeed * deltaTime;
    //         glm::vec3 center = planets[mPlayers[0].planetIndex]->GetCenter();
    //         // 空中回避：直前の高さ（惑星中心からの距離）を維持して浮遊
    //         float dist = glm::length(mPlayers[0]->GetPos() - center);
    //         if (dist > 1e-6f)
    //             mPlayers[0]->GetPos() = center + (mPlayers[0]->GetPos() - center) / dist * dodgeStartHeight;
    //         dodgeTimer -= deltaTime;
    //     }
    //     if (dodgeCooldown > 0.0f)
    //         dodgeCooldown -= deltaTime;

    //     // 攻撃後＋0.5秒：攻撃時の高さを維持して浮遊（空中固定）
    //     if (attackHeightLockRemaining > 0.0f)
    //     {
    //         glm::vec3 center = planets[mPlayers[0].planetIndex]->GetCenter();
    //         float dist = glm::length(mPlayers[0]->GetPos() - center);
    //         if (dist > 1e-6f)
    //             mPlayers[0]->GetPos() = center + (mPlayers[0]->GetPos() - center) / dist * attackStartHeight;
    //     }
    //     // Bullet レイキャスト：足元にメッシュがあれば地形に沿わせ、穴の上なら重力で落ちる
    //     // 回避中・攻撃硬直中は浮遊のためスキップ。上昇中（ジャンプ直後）もスキップし、地上 or 落下中のみ判定
    //     glm::vec3 upForJump = glm::normalize(mPlayers[0]->GetPos() - planets[mPlayers[0].planetIndex]->GetCenter());
    //     bool isRising = glm::dot(mPlayers[0].velocity, upForJump) > 0.5f;
    //     bool meshGround = false;
    //     if (dodgeTimer <= 0.0f && attackHeightLockRemaining <= 0.0f && bulletOk && bulletWorld && !isRising)
    //     {
    //         glm::vec3 center = planets[mPlayers[0].planetIndex]->GetCenter();
    //         glm::vec3 upDir = glm::normalize(mPlayers[0]->GetPos() - center);
    //         glm::vec3 rayFrom3 = mPlayers[0]->GetPos() + upDir * 0.1f;
    //         glm::vec3 rayTo3 = mPlayers[0]->GetPos() - upDir * 0.1f;
    //         btVector3 rayFrom(rayFrom3.x, rayFrom3.y, rayFrom3.z);
    //         btVector3 rayTo(rayTo3.x, rayTo3.y, rayTo3.z);
    //         btCollisionWorld::ClosestRayResultCallback rayCallback(rayFrom, rayTo);
    //         bulletWorld->rayTest(rayFrom, rayTo, rayCallback);
    //         if (rayCallback.hasHit())
    //         {
    //             btVector3 hitPt = rayCallback.m_hitPointWorld;
    //             glm::vec3 hitPos(hitPt.x(), hitPt.y(), hitPt.z());
    //             float hitDist = glm::length(hitPos - center);
    //             float playerDist = glm::length(mPlayers[0]->GetPos() - center);
    //             if (playerDist - hitDist < 2.0f)
    //             {
    //                 mPlayers[0]->GetPos() = hitPos;
    //                 mPlayers[0].onGround = true;
    //                 mPlayers[0].velocity = glm::vec3(0.0f);
    //                 meshGround = true;
    //             }
    //         }
    //         if (!meshGround && mPlayers[0].onGround)
    //         {
    //             mPlayers[0].onGround = false;
    //         }
    //     }
    //     // 回避中・攻撃空中固定中は重力をかけず、終了後に通常通り落下
    //     if (dodgeTimer <= 0.0f && attackHeightLockRemaining <= 0.0f)
    //         updatePlayerPhysics(mPlayers[0], deltaTime, planets, &transitionTimer, bulletOk && bulletWorld);

    //     // 落下して惑星内部にめり込んだらリスタート地点へ
    //     if (bulletOk && bulletWorld)
    //     {
    //         float dist = glm::length(mPlayers[0]->GetPos() - planets[mPlayers[0].planetIndex]->GetCenter());
    //         float r = planets[mPlayers[0].planetIndex]->GetRadius();
    //         if (dist < r * 0.5f)
    //         {
    //             mPlayers[0]->GetPos() = restartPos;
    //             mPlayers[0].planetIndex = restartPlanetIndex;
    //             mPlayers[0].velocity = glm::vec3(0.0f);
    //             mPlayers[0].onGround = true;
    //         }
    //     }
    // }

    // float planetRadius = planets[mPlayers[0].planetIndex]->GetRadius();
    // const float attackRangeMargin = 0.2f;
    // for (unique_ptr<Enemy> &ptr : enemies)
    // {
    //     EnemyBase &e = *ptr;
    //     if (!e.alive)
    //         continue;
    //     // 追跡：どちらかのプレイヤーに向かう（1P基準で判定）
    //     float distToP0 = glm::length(mPlayers[0]->GetPos() - e->GetPos());
    //     glm::vec3 toPlayer = glm::normalize(mPlayers[0]->GetPos() - e->GetPos());
    //     if (distToP0 <= EnemyBase::Sensing && e.damageTimer <= 0.0f && !mPlayers[0].isDamaged && distToP0 >= e.getRadius())
    //     {
    //         e->GetPos() += toPlayer * e.speed * deltaTime;
    //         float r = planets[e.planetIndex]->GetRadius();
    //         e->GetPos() = planets[e.planetIndex]->GetCenter() + glm::normalize(e->GetPos() - planets[e.planetIndex]->GetCenter()) * r;
    //     }
    //     // 攻撃範囲：いずれかのプレイヤーが範囲内なら「範囲内」とする（タイマーは敵ごとに1つなので二重更新しない）
    //     float distToP1 = glm::length(mPlayers[1]->GetPos() - e->GetPos());
    //     bool inRangeOfP0 = (distToP0 <= e.getRadius() + attackRangeMargin);
    //     bool inRangeOfP1 = (distToP1 <= e.getRadius() + attackRangeMargin);
    //     bool inRangeOfAny = inRangeOfP0 || inRangeOfP1;
    //     if (!inRangeOfAny)
    //     {
    //         e.standByAttackTimer = -1.0f; // 全員範囲外なら idle
    //     }
    //     else if (e.standByAttackTimer == -1.0f || e.standByAttackTimer == -2.0f)
    //     {
    //         if (!mPlayers[0].isDamagePrev && !mPlayers[1].isDamagePrev)
    //             e.standByAttackTimer = 2.0f; // 範囲内で idle/攻撃直後のときだけスタンバイ開始（1回だけ）
    //     }
    //     if (e.standByAttackTimer >= 0.0f)
    //     {
    //         float prevStandBy = e.standByAttackTimer;
    //         e.standByAttackTimer -= deltaTime;
    //         if (prevStandBy >= 0.5f && e.standByAttackTimer < 0.5f && attackPreSE)
    //         {
    //             Mix_PlayChannel(-1, attackPreSE, 0); // 敵の攻撃0.5秒前に鳴らす
    //         }
    //     }
    //     if (e.standByAttackTimer <= 0.0f && inRangeOfAny)
    //     {
    //         e.standByAttackTimer = -2.0f; // 攻撃終了（次のフレームで範囲内なら再スタンバイ可能）
    //         if (inRangeOfP0 && !mPlayers[0].isDamagePrev)
    //         {
    //             mPlayers[0].hp -= e.attack;
    //             mPlayers[0].damageTimer = 1.0f;
    //             mPlayers[0].isDamaged = true;
    //             playerKnockbackFrom = e->GetPos();
    //             if (mPlayers[0].hp <= 0)
    //             {
    //                 mPlayers[0].hp = 0;
    //                 mPlayers[0]->GetPos() = restartPos;
    //                 mPlayers[0].planetIndex = restartPlanetIndex;
    //                 mPlayers[0].velocity = glm::vec3(0.0f);
    //                 mPlayers[0].onGround = true;
    //             }
    //         }
    //         if (inRangeOfP1 && !mPlayers[1].isDamagePrev)
    //         {
    //             mPlayers[1].hp -= e.attack;
    //             mPlayers[1].damageTimer = 1.0f;
    //             mPlayers[1].isDamaged = true;
    //             playerKnockbackFrom = e->GetPos();
    //             if (mPlayers[1].hp <= 0)
    //             {
    //                 mPlayers[1].hp = 0;
    //                 mPlayers[1]->GetPos() = restartPos;
    //                 mPlayers[1].planetIndex = restartPlanetIndex;
    //                 mPlayers[1].velocity = glm::vec3(0.0f);
    //                 mPlayers[1].onGround = true;
    //             }
    //         }
    //     }
    // }

    // // Xボタン攻撃：正面にいる敵のうち最初の1体にダメージ（1Pのみ）。当たらなくても硬直・高さ維持する
    // const float attackRange = 1.8f;
    // const float attackAngle = 0.8f;
    // if (attackPressed && !attackPressedPrev && mPlayers[0].attackCooldownRemaining <= 0.0f)
    // {
    //     attackStartHeight = glm::length(mPlayers[0]->GetPos() - planets[mPlayers[0].planetIndex]->GetCenter());
    //     mPlayers[0].velocity = glm::vec3(0.0f);
    //     bool hitTarget = false;
    //     glm::vec3 attackFwd, attackLeftUnused;
    //     getForwardLeft(up, mPlayers[0]->GetFacingYaw(), attackFwd, attackLeftUnused);
    //     for (unique_ptr<Enemy> &ptr : enemies)
    //     {
    //         EnemyBase &e = *ptr;
    //         if (!e.alive)
    //             continue;
    //         glm::vec3 toEnemy = glm::normalize(e->GetPos() - mPlayers[0]->GetPos());
    //         float dot = glm::dot(-attackFwd, toEnemy);
    //         float dist = glm::length(e->GetPos() - mPlayers[0]->GetPos());
    //         float effectiveRange = attackRange + e.getRadius();
    //         if (dist <= effectiveRange && dot >= attackAngle)
    //         {
    //             e.hp -= mPlayers[0].attack;
    //             if (e.hp <= 0)
    //                 e.damageTimer = 1.0f; // 死ぬ時だけノックバック
    //             if (attackSE)
    //                 Mix_PlayChannel(-1, attackSE, 0);
    //             mPlayers[0].attackCooldownRemaining = 0.3f;
    //             mPlayers[0].attackIndex++;
    //             bool wasThirdHit = false;
    //             if (mPlayers[0].attackIndex == 3)
    //             {
    //                 wasThirdHit = true;
    //                 mPlayers[0].attackCooldownRemaining = 1.5f;
    //                 mPlayers[0].attackIndex = 0;
    //             }
    //             mPlayers[0].attackMoveLockRemaining = std::min(mPlayers[0].attackCooldownRemaining, 1.0f) + 0.5f;
    //             mPlayers[0].attackDodgeLockRemaining = std::max(0.0f, mPlayers[0].attackMoveLockRemaining - 0.5f);
    //             if (wasThirdHit && !mPlayers[0].onGround)
    //                 attackHeightLockRemaining = 0.5f; // 空中で最後の攻撃は0.5秒だけ浮遊してから落下
    //             else
    //                 attackHeightLockRemaining = mPlayers[0].attackMoveLockRemaining + 0.5f;
    //             hitTarget = true;
    //             if (e.hp <= 0)
    //             {
    //                 e.hp = 0;
    //             };
    //             break;
    //         }
    //     }
    //     if (!hitTarget)
    //     {
    //         mPlayers[0].attackCooldownRemaining = 0.3f;
    //         mPlayers[0].attackIndex++;
    //         bool wasThirdHit = false;
    //         if (mPlayers[0].attackIndex == 3)
    //         {
    //             wasThirdHit = true;
    //             mPlayers[0].attackCooldownRemaining = 1.5f;
    //             mPlayers[0].attackIndex = 0;
    //         }
    //         mPlayers[0].attackMoveLockRemaining = std::min(mPlayers[0].attackCooldownRemaining, 1.0f) + 0.5f;
    //         mPlayers[0].attackDodgeLockRemaining = std::max(0.0f, mPlayers[0].attackMoveLockRemaining - 0.5f);
    //         if (wasThirdHit && !mPlayers[0].onGround)
    //             attackHeightLockRemaining = 0.7f; // 空中で最後の攻撃は0.5秒だけ浮遊してから落下
    //         else
    //             attackHeightLockRemaining = mPlayers[0].attackMoveLockRemaining + 0.2f;
    //         if (attackMissSE)
    //             Mix_PlayChannel(-1, attackMissSE, 0);
    //     }
    // }

    // // L（L1）カウンター：敵のstandByAttackTimerが0.2f以下の時に押すと成功、counter.wav・ノックバック・攻撃力2倍ダメージ
    // if (counterPressed && !counterKeyPressedPrev && !boatTransitionActive)
    // {
    //     glm::vec3 counterFwd, counterLeftUnused;
    //     getForwardLeft(up, mPlayers[0]->GetFacingYaw(), counterFwd, counterLeftUnused);
    //     for (unique_ptr<Enemy> &ptr : enemies)
    //     {
    //         EnemyBase &e = *ptr;
    //         if (!e.alive)
    //             continue;
    //         if (e.standByAttackTimer <= 0.0f || e.standByAttackTimer > 0.2f)
    //             continue;
    //         glm::vec3 toEnemy = glm::normalize(e->GetPos() - mPlayers[0]->GetPos());
    //         float dot = glm::dot(-counterFwd, toEnemy);
    //         float dist = glm::length(e->GetPos() - mPlayers[0]->GetPos());
    //         float effectiveRange = attackRange + e.getRadius();
    //         if (dist <= effectiveRange && dot >= attackAngle)
    //         {
    //             if (counterSE)
    //                 Mix_PlayChannel(-1, counterSE, 0);
    //             e.standByAttackTimer = -1.0f;     // 敵の攻撃をキャンセル
    //             e.damageTimer = 1.0f;             // ノックバック
    //             e.hp -= 2.0f * mPlayers[0].attack; // 攻撃力2倍ダメージ
    //             if (e.hp <= 0)
    //                 e.hp = 0;
    //             break;
    //         }
    //     }
    // }

    // if (mPlayers[0].attackCooldownRemaining >= 0.0f)
    // {
    //     mPlayers[0].attackCooldownRemaining -= deltaTime;
    // }
    // if (mPlayers[0].attackMoveLockRemaining > 0.0f)
    // {
    //     mPlayers[0].attackMoveLockRemaining -= deltaTime;
    //     if (mPlayers[0].attackMoveLockRemaining < 0.0f)
    //         mPlayers[0].attackMoveLockRemaining = 0.0f;
    //     if (mPlayers[0].attackMoveLockRemaining <= 0.0f)
    //         mPlayers[0].attackIndex = 0; // 歩けるようになったらコンボリセット
    // }
    // if (mPlayers[0].attackDodgeLockRemaining > 0.0f)
    // {
    //     mPlayers[0].attackDodgeLockRemaining -= deltaTime;
    //     if (mPlayers[0].attackDodgeLockRemaining < 0.0f)
    //         mPlayers[0].attackDodgeLockRemaining = 0.0f;
    // }
    // if (attackHeightLockRemaining > 0.0f)
    // {
    //     attackHeightLockRemaining -= deltaTime;
    //     if (attackHeightLockRemaining < 0.0f)
    //         attackHeightLockRemaining = 0.0f;
    // }

    // 以下はまだ
    // for (unique_ptr<Enemy> &ptr : enemies)
    // {
    //     EnemyBase &e = *ptr;
    //     if (e.damageTimer > 0.0f)
    //     {
    //         glm::vec3 toEnemy = glm::normalize(e->GetPos() - mPlayers[0]->GetPos());
    //         e->GetPos() += toEnemy * deltaTime;
    //         e->GetPos() = planets[e.planetIndex]->GetCenter() + glm::normalize(e->GetPos() - planets[e.planetIndex]->GetCenter()) * planets[e.planetIndex]->GetRadius();
    //         e.damageTimer -= deltaTime;
    //     }
    //     else
    //     {
    //         if (e.hp <= 0)
    //         {
    //             e.hp = 0;
    //             e.alive = false;
    //             // 現在の惑星で倒した敵の位置を記録（鍵出現位置に使う）
    //             if (e.planetIndex == mPlayers[0].planetIndex)
    //                 lastDefeatedEnemyPos = e->GetPos();
    //             // ボス撃破でスター出現（撃破前のボスがいた場所に置く）
    //             if (e.isBoss() && !starVisibleFromBoss &&
    //                 e.planetIndex >= 0 && static_cast<size_t>(e.planetIndex) < planets.size())
    //             {
    //                 starVisibleFromBoss = true;
    //                 starBossPlanetIndex = e.planetIndex;
    //                 starPos = e->GetPos(); // ボスがいた場所
    //             }
    //         }
    //     }
    // }
    // ここまで

    // if (mPlayers[0].damageTimer > 0.0f)
    // {
    //     glm::vec3 toPlayer = glm::normalize(mPlayers[0]->GetPos() - playerKnockbackFrom);
    //     mPlayers[0]->GetPos() += toPlayer * deltaTime;
    //     mPlayers[0].damageTimer -= deltaTime;
    // }
    // else
    // {
    //     mPlayers[0].isDamaged = false;
    // }

    // // 今いる惑星の敵を全て倒したら鍵を出現させる
    // std::vector<EnemyBase *> currentPlanetEnemies;
    // for (const unique_ptr<Enemy> &p : enemies)
    // {
    //     if (p->planetIndex == mPlayers[0].planetIndex)
    //         currentPlanetEnemies.emplace_back(p.get());
    // }
    // bool allEnemiesDead = true;
    // for (EnemyBase *e : currentPlanetEnemies)
    // {
    //     if (e->alive)
    //     {
    //         allEnemiesDead = false;
    //         break;
    //     }
    // }
    // if (allEnemiesDead && !keyVisible && !keyObtained)
    // {
    //     keyVisible = true;
    //     keyPlanetIndex = mPlayers[0].planetIndex;
    //     keyPos = lastDefeatedEnemyPos; // 最後に倒した敵の場所に鍵を出す
    // }

    // // 鍵に触れたら取得して消す＆ボートを出現させる
    // if (keyVisible)
    // {
    //     float distToKey = glm::length(mPlayers[0]->GetPos() - keyPos);
    //     if (distToKey < keyPickupRadius)
    //     {
    //         keyVisible = false;
    //         keyObtained = true;
    //         int curIdx = mPlayers[0].planetIndex;
    //         boatSpawnPlanetIndex = curIdx;
    //         // 現在惑星から一番近い別惑星を到着先に
    //         float nearestDist = 1e30f;
    //         for (size_t i = 0; i < planets.size(); i++)
    //         {
    //             if (static_cast<int>(i) == curIdx)
    //                 continue;
    //             float d = glm::length(planets[i]->GetCenter() - planets[curIdx]->GetCenter());
    //             if (d < nearestDist)
    //             {
    //                 nearestDist = d;
    //                 boatDestinationPlanetIndex = static_cast<int>(i);
    //             }
    //         }
    //         // ボートを現在惑星の表面近くに配置
    //         float boatHeight = planets[curIdx]->GetRadius() - 0.15f;
    //         boatPos = planets[curIdx]->GetCenter() + glm::normalize(glm::vec3(0.0f, -1.0f, 0.5f)) * boatHeight;
    //     }
    // }

    // 以下はまだ
    // // ボートに触れたら到着先へ移動開始（ボートが出現した惑星にいる時のみ）
    // if (!boatTransitionActive && keyObtained && boatSpawnPlanetIndex >= 0 && boatDestinationPlanetIndex >= 0 && mPlayers[0].planetIndex == boatSpawnPlanetIndex)
    // {
    //     float distToBoat = glm::length(mPlayers[0]->GetPos() - boatPos);
    //     if (distToBoat < boatTouchRadius)
    //     {
    //         boatTransitionActive = true;
    //         boatTransitionTimer = 0.0f;
    //         boatTransitionStartBoat = boatPos;
    //         const Planet &dest = planets[boatDestinationPlanetIndex];
    //         const Planet &from = planets[boatSpawnPlanetIndex];
    //         glm::vec3 toDest = glm::normalize(dest->GetCenter() - from->GetCenter());
    //         boatTransitionEnd = dest->GetCenter() - toDest * dest->GetRadius();
    //     }
    // }

    // // スターに触れたらゲームクリア（ボス撃破で出現したスターが存在するときだけ判定）
    // bool starExists = (starVisibleFromBoss && mPlayers[0].planetIndex == starBossPlanetIndex);
    // if (!gameClear && starExists)
    // {
    //     float distToStar = glm::length(mPlayers[0]->GetPos() - starPos);
    //     if (distToStar < starTouchRadius)
    //     {
    //         gameClear = true;
    //         Mix_HaltMusic();
    //         if (clearSE)
    //             Mix_PlayChannel(-1, clearSE, 0);
    //         std::cout << "Game Clear!" << std::endl;
    //     }
    // }
}

void Game::GenerateOutput()
{
    glm::mat4 view = mPlayers[0]->getPlayerView();
    glm::mat4 view2P = mIsPlayer2Joined ? mPlayers[1]->getPlayerView() : view;

    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // // これから描画するときにどのプログラムを使うのか設定
    glUseProgram(mShader->GetShaderProgram());

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(mWindow, &fbWidth, &fbHeight);

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
                std::cerr << "Mesh empty" << std::endl;
                // glBindVertexArray(sphereVAO);
                // glUniform1i(locUseTexture, 0);
                // glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
            }
        }

        auto drawCharacter = [&](const glm::vec3 &pos, float scale, const glm::vec3 &fallbackColor,
                                 const glm::vec3 &up, float yaw, const std::vector<LoadedMesh> &meshes,
                                 const glm::vec3 *colorOverride = nullptr)
        {
            glm::vec3 fwd, left;
            mPlayers[0]->getForwardLeft(up, yaw, fwd, left);
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
                std::cerr << "Mesh empty" << std::endl;
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
        drawCharacter(mPlayers[0]->GetPos(), playerScale, glm::vec3(0.0f, 0.0f, 1.0f), up0, mPlayers[0]->GetFacingYaw(), mPlayers[0]->GetPlayerMeshes());

        // 2Pの描画
        if (mIsPlayer2Joined)
        {
            glm::vec3 up1 = glm::normalize(mPlayers[1]->GetPos() - planets[mPlayers[1]->GetCurrentPlanetNum()]->GetCenter());
            drawCharacter(mPlayers[1]->GetPos(), playerScale, glm::vec3(1.0f, 0.5f, 0.0f), up1, mPlayers[1]->GetFacingYaw(),  mPlayers[1]->GetPlayerMeshes());
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
            glm::vec3 enemyUp = glm::normalize(enemy->GetPos() - planets[enemy->GetCurrentPlanet()]->GetCenter());
            glm::vec3 toPlayer = glm::normalize(mPlayers[0]->GetPos() - enemy->GetPos());
            float enemyFacingYaw = mPlayers[0]->getYawFromDirection(enemyUp, toPlayer) + 3.14159265f;
            drawCharacter(enemy->GetPos(), enemy->GetScale(), glm::vec3(0.0f, 1.0f, 0.0f), enemyUp, enemyFacingYaw, eit->second);
            // 敵の頭上にID（1始まり）をビルボード表示
            // if (mFont)
            // {
            //     auto [texId, texSize] = getTextTexture(std::to_string(ei + 1));
            //     if (texId != 0 && texSize.x > 0 && texSize.y > 0)
            //     {
            //         glm::vec3 camPos(glm::inverse(viewMat)[3]);
            //         glm::vec3 quadCenter = e->GetPos() + enemyUp * 0.8f;
            //         glm::vec3 forward = glm::normalize(camPos - quadCenter);
            //         glm::vec3 right = glm::normalize(glm::cross(enemyUp, forward));
            //         if (glm::length(right) < 0.01f)
            //             right = glm::normalize(glm::cross(enemyUp, glm::vec3(0, 0, 1)));
            //         glm::vec3 upQuad = glm::cross(forward, right);
            //         float w = enemyLabelHeight * static_cast<float>(texSize.x) / static_cast<float>(texSize.y);
            //         glm::mat4 billboard(1.0f);
            //         billboard[0] = glm::vec4(right * w, 0.0f);
            //         billboard[1] = glm::vec4(upQuad * enemyLabelHeight, 0.0f);
            //         billboard[2] = glm::vec4(forward, 0.0f);
            //         billboard[3] = glm::vec4(quadCenter, 1.0f);
            //         glEnable(GL_BLEND);
            //         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            //         glDepthMask(GL_FALSE);
            //         glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(billboard));
            //         glActiveTexture(GL_TEXTURE0);
            //         glBindTexture(GL_TEXTURE_2D, texId);
            //         glUniform1i(locUseTexture, 1);
            //         glUniform3f(locObjectColor, 1.0f, 1.0f, 1.0f);
            //         glBindVertexArray(textQuadVAO);
            //         glDrawArrays(GL_TRIANGLES, 0, 6);
            //         glUniform1i(locUseTexture, 0);
            //         glDepthMask(GL_TRUE);
            //         glDisable(GL_BLEND);
            //     }
            // }
        }

        // // 鍵描画（出現した惑星上で表示）
        // if (keyVisible && keyPlanetIndex >= 0 && static_cast<size_t>(keyPlanetIndex) < planets.size())
        // {
        //     glm::vec3 keyUp = glm::normalize(keyPos - planets[keyPlanetIndex]->GetCenter());
        //     drawCharacter(keyPos, keyScale, keyColor, keyUp, 0.0f, keyMeshes, &keyColor);
        // }

        // // ボート描画（ボートが出現した惑星にいる時か移動中のみ表示）
        // if (keyObtained && boatSpawnPlanetIndex >= 0 &&
        //     (mPlayers[0].planetIndex == boatSpawnPlanetIndex || boatTransitionActive))
        // {
        //     int drawPlanet = boatTransitionActive ? 0 : boatSpawnPlanetIndex;
        //     if (boatTransitionActive)
        //     {
        //         float nearestD = glm::length(boatPos - planets[0]->GetCenter());
        //         for (size_t i = 1; i < planets.size(); i++)
        //         {
        //             float d = glm::length(boatPos - planets[i]->GetCenter());
        //             if (d < nearestD)
        //             {
        //                 nearestD = d;
        //                 drawPlanet = static_cast<int>(i);
        //             }
        //         }
        //     }
        //     glm::vec3 boatUp = glm::normalize(boatPos - planets[drawPlanet]->GetCenter());
        //     drawCharacter(boatPos, boatScale, glm::vec3(0.4f, 0.25f, 0.1f), boatUp, 0.0f, boatMeshes);
        // }

        // // スター描画（ボス撃破後のみ存在・描画）
        // if (!gameClear)
        // {
        //     int starPlanetIdx = -1;
        //     if (starVisibleFromBoss && starBossPlanetIndex >= 0 && static_cast<size_t>(starBossPlanetIndex) < planets.size())
        //         starPlanetIdx = starBossPlanetIndex;
        //     if (starPlanetIdx >= 0)
        //     {
        //         glm::vec3 starUp = glm::normalize(starPos - planets[starPlanetIdx]->GetCenter());
        //         glm::vec3 starColor(1.0f, 0.9f, 0.2f);
        //         drawCharacter(starPos, starScale, starColor, starUp, 0.0f, starMeshes);
        //     }
        // }
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

    // // バッファーを入れ替える
    glfwSwapBuffers(mWindow);
}

void Game::AddActor(std::unique_ptr<Actor> actor)
{
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