// 学習用にコメントをつけています。
#include <GL/glew.h>
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
#include "./shader.h"
#include "./model_loader.h"
#include "./planet.h"
#include "./player.h"
#include "./enemy.h"
#include "./enemy_loader.h"
#include "./planet_loader.h"
#include "./player_loader.h"
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// #region agent log
static const char* const DEBUG_LOG_PATH = "/Users/kusakatakumi/Desktop/desktop_latest/engine/.cursor/debug-a01f92.log";
static void debug_log(const char* hypothesisId, const char* message, const std::string& dataJson) {
    std::ofstream f(DEBUG_LOG_PATH, std::ios::app);
    if (!f) return;
    f << "{\"sessionId\":\"a01f92\",\"hypothesisId\":\"" << hypothesisId << "\",\"location\":\"main.cpp\",\"message\":\"" << message << "\",\"data\":" << dataJson << ",\"timestamp\":" << (unsigned long long)(glfwGetTime() * 1000.0) << "}\n";
    f.close();
}
// #endregion

int main() {
    // glfwの初期化
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW" << std::endl;
        return -1;
    }
    // OpenGLのバージョンを3.3に設定
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// OpenGLのプロファイルをCoreプロファイルにする
	// 学習用・・・プロファイルは2種類ある（CompatibilityがglBeginなどの古い処理も使える,Coreはシェーダー必須で古いのは使えない）
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
    // メインのモニタを取得
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    // ゲーム用のウィンドウを作成する
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Engine", monitor, nullptr);

    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // glewの初期化（これがないとOpenGLのシェーダー周りなどの関数が使えない）
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to init GLEW" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    // 2つのシェーダーをリンクしてシェーダープログラムを作成する
    unsigned int shaderProgram = createShaderProgram("../shaders/vertex.glsl", "../shaders/fragment.glsl");
    if (!shaderProgram) {
        glfwTerminate();
        return -1;
    }

    // コントローラー設定
    SDL_GameController* sdlController = nullptr;
    // SDLのゲームパッド用サブシステムを有効にする
    if (SDL_Init(SDL_INIT_GAMECONTROLLER) == 0) {
        // SDL_ttfの初期化
        if (TTF_Init() != 0)
            std::cerr << "TTF_Init failed: " << TTF_GetError() << std::endl;
        // 今つながっているコントローラーの数を取得してループ
        for (int i = 0; i < SDL_NumJoysticks(); ++i) {
            // i番目がゲームコントローラーとして認識できるか判定
            if (SDL_IsGameController(i)) {
                // それをゲームコントローラーとして開く
                sdlController = SDL_GameControllerOpen(i);
            }
        }
    }

    // SDL_mixtureの初期化（オーディオを開く）
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) != 0) {
        std::cerr << "Mix_OpenAudio error: " << Mix_GetError() << std::endl;
    }
    
    // BGMをロードする
    Mix_Music* bgm = Mix_LoadMUS("../assets/audio/bgm.wav");
    if (!bgm) {
        std::cerr << "Mix_LoadMUS error: " << Mix_GetError() << std::endl;
    } else {
        // BGMをループ再生
        Mix_PlayMusic(bgm, -1);
    }
    Mix_Music* bgmBoss = Mix_LoadMUS("../assets/audio/boss.wav");
    if (!bgmBoss) {
        std::cerr << "Mix_LoadMUS (boss BGM) error: " << Mix_GetError() << std::endl;
    }

    // 攻撃ヒットSE（SDL_mixer の SE は WAV 形式。attack.mp3 のみの場合は WAV に変換して attack.wav を用意）
    Mix_Chunk* attackSe = Mix_LoadWAV("../assets/audio/attack.wav");
    if (!attackSe) {
        attackSe = Mix_LoadWAV("../assets/audio/attack.mp3");
    }
    if (!attackSe) {
        std::cerr << "Mix_LoadWAV (attack SE) error: " << Mix_GetError() << std::endl;
    }
    Mix_Chunk* attackMissSe = Mix_LoadWAV("../assets/audio/attack_miss.wav");
    if (!attackMissSe) {
        std::cerr << "Mix_LoadWAV (attack_miss SE) error: " << Mix_GetError() << std::endl;
    }
    Mix_Chunk* attackPreSe = Mix_LoadWAV("../assets/audio/attack_pre.wav");
    if (!attackPreSe) {
        std::cerr << "Mix_LoadWAV (attack_pre SE) error: " << Mix_GetError() << std::endl;
    }
    Mix_Chunk* counterSe = Mix_LoadWAV("../assets/audio/counter.wav");
    if (!counterSe) {
        std::cerr << "Mix_LoadWAV (counter SE) error: " << Mix_GetError() << std::endl;
    }
    Mix_Chunk* clearSe = Mix_LoadWAV("../assets/audio/clear.wav");
    if (!clearSe) {
        std::cerr << "Mix_LoadWAV (clear SE) error: " << Mix_GetError() << std::endl;
    }

    // 描画する三角形の3頂点の座標（プレイヤー用フォールバック）
    std::vector<float> fallbackTriangleVertices;
    getPlayerFallbackTriangle(fallbackTriangleVertices);

    // VBOはデータ（座標の位置など）を入れるもの、VAOはその使い方などの設定
    unsigned int VBO, VAO;
    constexpr GLuint A_POSITION_LOCATION = 0;
    // VAO用の空のオブジェクトを作ってそのIDをVAOに格納
    glGenVertexArrays(1, &VAO);
    // VBO用の空のオブジェクトを作ってそのIDをVBOに格納
    glGenBuffers(1, &VBO);
    // 今後の設定をどのVAOまたはVBOにするのかを設定（カレント）
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // GPU上のバッファにvertices（CPU側の配列）をコピーしてGPUで読めるようにする
    glBufferData(GL_ARRAY_BUFFER, fallbackTriangleVertices.size() * sizeof(float), fallbackTriangleVertices.data(), GL_STATIC_DRAW);
    // VBO上のデータを3つ区切りで読んでlocation 0に渡すことを設定している
    glVertexAttribPointer(A_POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // location 0をオンにする
    glEnableVertexAttribArray(A_POSITION_LOCATION);
    // unbindする
    glBindVertexArray(0);

    // 球の頂点座標を格納する配列
    std::vector<float> sphereVertices;
    // どの頂点で三角形を作るかを表すインデックスの配列
    std::vector<unsigned int> sphereIndices;
    buildSphereMesh(18, 18, 1.0f, sphereVertices, sphereIndices);

    // 球用のVAOなどを用意する
    unsigned int sphereVAO, sphereVBO, sphereEBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(A_POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(A_POSITION_LOCATION);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);

    const unsigned int sphereIndexCount = static_cast<unsigned int>(sphereIndices.size());
    // プレイヤーモデルをロードする
    std::vector<LoadedMesh> playerMeshes = loadMeshFromFile("../assets/models/player.obj");
    // 敵モデルをロードする
    // 敵モデルは enemies 読み込み後に modelPath ごとにロードする（下で実施）
    // 鍵モデルをロードする（敵全員撃破で出現）
    std::vector<LoadedMesh> keyMeshes = loadMeshFromFile("../assets/models/key.obj");
    // ボートモデル（鍵取得で惑星近くに出現）
    std::vector<LoadedMesh> boatMeshes = loadMeshFromFile("../assets/models/boat.obj");
    // スター（惑星2に配置、触れるとゲームクリア）
    std::vector<LoadedMesh> starMeshes = loadMeshFromFile("../assets/models/star.obj");

    // フォント
    TTF_Font* font = nullptr;
    const char* fontPaths[] = {
        "../assets/fonts/font.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/Library/Fonts/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    };
    // 最初に見つかったフォントを用いる
    for (const char* path : fontPaths) {
        font = TTF_OpenFont(path, 24);
        if (font) break;
    }
    // フォント失敗処理
    if (!font)
        std::cerr << "No font loaded. Enemy ID labels will not be shown." << std::endl;
    std::unordered_map<std::string, std::pair<GLuint, glm::ivec2>> textTextureCache;
    // 敵のどれくらい上にラベルを描画するのか
    const float enemyLabelHeight = 0.5f;

    // テキスト用四角形
    unsigned int textQuadVAO, textQuadVBO;
    {
        float quad[] = {
            -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
             0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
             0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
            -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
             0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
            -0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
        };
        glGenVertexArrays(1, &textQuadVAO);
        glGenBuffers(1, &textQuadVBO);
        glBindVertexArray(textQuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, textQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }

    // シェーダープログラム内のMVPのIDを探して格納
    GLint locModel = glGetUniformLocation(shaderProgram, "model");
    GLint locView = glGetUniformLocation(shaderProgram, "view");
    GLint locProj = glGetUniformLocation(shaderProgram, "projection");
    GLint locObjectColor = glGetUniformLocation(shaderProgram, "objectColor");
    GLint locUseTexture = glGetUniformLocation(shaderProgram, "useTexture");
    GLint locDiffuseTexture = glGetUniformLocation(shaderProgram, "diffuseTexture");

    // 惑星をYAMLから読み込み
    static const char* const planetsYamlPath = "../assets/data/planets.yaml";
    std::vector<Planet> planets;
    if (!loadPlanetsFromYaml(planetsYamlPath, planets)) {
        planets = {
            { glm::vec3(0.0f, 0.0f, 0.0f), 8.0f, glm::vec3(1.0f, 0.0f, 0.0f) },
            { glm::vec3(32.0f, 0.0f, 0.0f), 10.0f, glm::vec3(0.0f, 1.0f, 0.0f) },
        };
        std::cerr << "Planet YAML load failed, using default 2 planets." << std::endl;
    }

    // 惑星ごとのモデルをロード
    std::unordered_map<std::string, std::vector<LoadedMesh>> planetMeshesByPath;
    for (const Planet& planet : planets) {
        if (planetMeshesByPath.find(planet.modelPath) == planetMeshesByPath.end()) {
            std::string path = "../assets/models/" + planet.modelPath;
            planetMeshesByPath[planet.modelPath] = loadMeshFromFile(path.c_str());
        }
    }

    const float characterSpeed = 2.0f;
    const float cameraDistance = 12.0f;
    const float cameraSensitivity = 2.5f;

    // 1P/2P 状態（players[0]=1P, players[1]=2P）
    PlayerState players[2];
    static const char* const playersYamlPath = "../assets/data/players.yaml";
    PlayerInitialConfig playerInitial;
    if (loadPlayerInitialFromYaml(playersYamlPath, planets, playerInitial)) {
        players[0].pos = playerInitial.pos;
        players[0].planetIndex = playerInitial.planetIndex;
        players[0].cameraPitch = playerInitial.cameraPitch;
        players[0].attack = playerInitial.attack;
        players[0].hp = playerInitial.hp;
    } else {
        players[0].pos = glm::vec3(8.0f, planets[0].radius, 2.5f);
        players[0].planetIndex = 0;
        players[0].cameraPitch = 0.4f;
        players[0].attack = 10.0f;
        players[0].hp = 100.0f;
        std::cerr << "Player YAML load failed, using default 1P config." << std::endl;
    }
    glm::vec3 restartPos = players[0].pos;
    int restartPlanetIndex = 0;
    bool player2Joined = false;

    static const char* const enemiesYamlPath = "../assets/data/enemies.yaml";
    // 敵を YAML から読み込み（失敗時は1体のデフォルト）
    std::vector<EnemyPtr> enemies;
    if (!loadEnemiesFromYaml(enemiesYamlPath, planets, enemies)) {
        auto def = std::make_unique<NormalEnemy>();
        glm::vec3 center = planets[0].center;
        float radius = planets[0].radius;
        def->pos = center + radius * glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f));
        def->planetIndex = 0;
        def->hp = 10.0f;
        def->modelPath = "enemy.obj";
        def->drawScale = 0.25f;
        def->speed = 2.0f;
        def->attack = 20.0f;
        enemies.push_back(std::move(def));
        std::cerr << "Enemy YAML load failed, using 1 default enemy." << std::endl;
    }
    // 敵ごとのモデルをロード（同じ modelPath は1回だけ）
    std::unordered_map<std::string, std::vector<LoadedMesh>> enemyMeshesByPath;
    for (const EnemyPtr& p : enemies) {
        const std::string& pathKey = p->modelPath;
        if (enemyMeshesByPath.find(pathKey) == enemyMeshesByPath.end()) {
            std::string path = "../assets/models/" + pathKey;
            enemyMeshesByPath[pathKey] = loadMeshFromFile(path.c_str());
        }
    }
    glm::vec3 playerKnockbackFrom(0.0f);

    // 鍵（現在の惑星で最後に倒した敵の場所に出現）
    bool keyVisible = false;
    bool keyObtained = false;
    glm::vec3 keyPos(0.0f);
    int keyPlanetIndex = -1;
    glm::vec3 lastDefeatedEnemyPos(0.0f);  // 現在惑星で最後に倒した敵の位置
    const float keyScale = 2.0f;
    const float keyPickupRadius = 1.2f;
    const glm::vec3 keyColor(0.85f, 0.65f, 0.13f);  // 金色

    // ボート
    glm::vec3 boatPos(0.0f);
    const float boatScale = 0.8f;
    const float boatTouchRadius = 1.8f;
    const float playerHeightAboveBoat = 0.7f;

    // ボートで次の惑星へ移動（触れたら少しずつ移動して到着）
    bool boatTransitionActive = false;
    float boatTransitionTimer = 0.0f;
    const float boatTransitionDuration = 5.0f;
    glm::vec3 boatTransitionStartBoat, boatTransitionEnd;
    int boatSpawnPlanetIndex = -1;
    int boatDestinationPlanetIndex = -1;
    int currentBgmPlanetIndex = -1;  // 現在流しているBGMの惑星（惑星2ならboss.wav）

    // スター（鍵ルートの到着先 or ボス撃破で出現）
    glm::vec3 starPos(0.0f);
    const float starScale = 0.3f;
    const float starTouchRadius = 1.5f;
    bool gameClear = false;
    bool starVisibleFromBoss = false;  // ボス撃破でスター出現したか
    int starBossPlanetIndex = -1;      // ボス撃破で出たスターの惑星番号

    // 時間情報
    double lastTime = glfwGetTime();
    float transitionTimer = 0.0f;
    float damageTimer = 0.0f;
    bool attackPressedPrev = false;
    bool dodgePressedPrev = false;
    bool reloadKeyPressedPrev = false;
    bool counterKeyPressedPrev = false;
    float dodgeCooldown = 0.0f;
    float dodgeTimer = 0.0f;
    glm::vec3 dodgeDir(0.0f);
    float dodgeStartHeight = 0.0f;  // 空中回避時の惑星中心からの距離（浮遊高さ）
    float attackStartHeight = 0.0f;   // 攻撃硬直時の惑星中心からの距離（浮遊高さ）
    float attackHeightLockRemaining = 0.0f;  // 攻撃後の空中固定時間（硬直＋0.5秒）
    const float dodgeDistance = 3.0f;
    const float dodgeDuration = 0.5f;
    const float dodgeCooldownTime = 1.0f;

    // 深度テストをONにして奥行きに応じて描画できるようにする（描画順ではなく、手前にあるものが上書きされて描画される）
    glEnable(GL_DEPTH_TEST);

    // Bullet Physics：惑星メッシュの当たり判定（惑星ごとに modelPath のメッシュを使用）
    bool bulletOk = false;
    btDefaultCollisionConfiguration* bulletCollisionConfig = nullptr;
    btCollisionDispatcher* bulletDispatcher = nullptr;
    btBroadphaseInterface* bulletBroadphase = nullptr;
    btSequentialImpulseConstraintSolver* bulletSolver = nullptr;
    btDiscreteDynamicsWorld* bulletWorld = nullptr;
    std::vector<btTriangleMesh*> bulletPlanetMeshes;
    std::vector<btBvhTriangleMeshShape*> bulletPlanetShapes;
    std::vector<btRigidBody*> bulletPlanetBodies;
    btPairCachingGhostObject* bulletGhost = nullptr;
    btCapsuleShape* bulletCapsule = nullptr;
    btSphereShape* bulletWallSphere = nullptr;  // 壁当たり用スイープ
    btKinematicCharacterController* bulletCharController = nullptr;

    if (!planets.empty()) {
        bulletCollisionConfig = new btDefaultCollisionConfiguration();
        bulletDispatcher = new btCollisionDispatcher(bulletCollisionConfig);
        bulletBroadphase = new btDbvtBroadphase();
        bulletSolver = new btSequentialImpulseConstraintSolver();
        bulletWorld = new btDiscreteDynamicsWorld(bulletDispatcher, bulletBroadphase, bulletSolver, bulletCollisionConfig);
        bulletWorld->setGravity(btVector3(0, -9.8f, 0));
        bulletBroadphase->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());

        std::vector<float> pos;
        std::vector<unsigned int> idx;
        for (size_t p = 0; p < planets.size(); p++) {
            pos.clear();
            idx.clear();
            std::string meshPath = "../assets/models/" + planets[p].modelPath;
            if (!loadMeshPositionsAndIndices(meshPath.c_str(), pos, idx) || pos.size() < 9 || idx.size() < 3)
                continue;
            const glm::vec3& center = planets[p].center;
            float radius = planets[p].radius;
            btTriangleMesh* triMesh = new btTriangleMesh();
            for (size_t i = 0; i + 2 < idx.size(); i += 3) {
                unsigned int i0 = idx[i], i1 = idx[i + 1], i2 = idx[i + 2];
                if (i0 * 3 + 2 >= pos.size() || i1 * 3 + 2 >= pos.size() || i2 * 3 + 2 >= pos.size()) continue;
                btVector3 v0(center.x + radius * pos[i0 * 3], center.y + radius * pos[i0 * 3 + 1], center.z + radius * pos[i0 * 3 + 2]);
                btVector3 v1(center.x + radius * pos[i1 * 3], center.y + radius * pos[i1 * 3 + 1], center.z + radius * pos[i1 * 3 + 2]);
                btVector3 v2(center.x + radius * pos[i2 * 3], center.y + radius * pos[i2 * 3 + 1], center.z + radius * pos[i2 * 3 + 2]);
                triMesh->addTriangle(v0, v1, v2);
            }
            bulletPlanetMeshes.push_back(triMesh);
            // #region agent log
            { std::ostringstream ds; ds << "{\"planet\":" << p << ",\"numTriangles\":" << triMesh->getNumTriangles() << "}"; debug_log("E", "planet_mesh_built", ds.str()); }
            // #endregion
            btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(triMesh, true);
            bulletPlanetShapes.push_back(shape);
            btTransform startTransform;
            startTransform.setIdentity();
            startTransform.setOrigin(btVector3(0, 0, 0));
            btRigidBody::btRigidBodyConstructionInfo rbInfo(0, nullptr, shape);
            btRigidBody* body = new btRigidBody(rbInfo);
            body->setWorldTransform(startTransform);
            body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
            bulletWorld->addRigidBody(body, (short)btBroadphaseProxy::DefaultFilter, (short)-1);
            bulletPlanetBodies.push_back(body);
        }
        bulletOk = !bulletPlanetBodies.empty();
        // #region agent log
        { debug_log("A", "collision_setup", "{\"ghostGroup\":\"CharacterFilter\",\"ghostMask\":\"-1\",\"staticGroup\":\"DefaultFilter\",\"staticMask\":\"-1\",\"ghostSpawnOffset\":true}"); }
        // #endregion

        const float capRadius = 0.35f;
        const float capHeight = 0.8f;
        bulletCapsule = new btCapsuleShape(capRadius, capHeight);
        bulletWallSphere = new btSphereShape(0.35f);  // 壁スイープ用（キャラ半径程度）
        bulletGhost = new btPairCachingGhostObject();
        bulletGhost->setCollisionShape(bulletCapsule);
        bulletGhost->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
        btTransform ghostTrans;
        ghostTrans.setIdentity();
        glm::vec3 spawnUp = glm::normalize(players[0].pos - planets[0].center);
        float capHalf = capHeight * 0.5f;
        glm::vec3 ghostOrigin = players[0].pos + spawnUp * (capHalf + 0.15f);
        ghostTrans.setOrigin(btVector3(ghostOrigin.x, ghostOrigin.y, ghostOrigin.z));
        bulletGhost->setWorldTransform(ghostTrans);
        bulletWorld->addCollisionObject(bulletGhost, (short)btBroadphaseProxy::CharacterFilter, (short)-1);
        bulletCharController = new btKinematicCharacterController(bulletGhost, bulletCapsule, 0.35f);
        bulletCharController->setGravity(btVector3(0, -9.8f, 0));
        bulletCharController->setJumpSpeed(5.0f);
        bulletCharController->setFallSpeed(55.0f);
        bulletWorld->addAction(bulletCharController);
    } else {
        if (!bulletOk) std::cerr << "Bullet: planet mesh load failed, using sphere collision." << std::endl;
        bulletOk = false;
    }

    // 文字列→テクスチャ（敵ID表示用、キャッシュ付き）
    auto getTextTexture = [&](const std::string& s) -> std::pair<GLuint, glm::ivec2> {
        if (!font || s.empty()) return {0, {0, 0}};
        auto it = textTextureCache.find(s);
        if (it != textTextureCache.end()) return it->second;
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* surf = TTF_RenderText_Blended(font, s.c_str(), white);
        if (!surf) return {0, {0, 0}};
        SDL_Surface* rgba = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(surf);
        if (!rgba) return {0, {0, 0}};
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

	// ゲームループ
    while (!glfwWindowShouldClose(window)) {
        // 入力などのイベントを処理する
        glfwPollEvents();
        double currentTime = glfwGetTime();
        // 前フレームからの経過時間を計算
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        glm::vec3 up = glm::normalize(players[0].pos - planets[players[0].planetIndex].center);

        // 敵データのホットリロード
        {
            bool reloadPressed = (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS);
            if (reloadPressed && !reloadKeyPressedPrev) {
                std::vector<EnemyPtr> newEnemies;
                if (loadEnemiesFromYaml(enemiesYamlPath, planets, newEnemies)) {
                    enemies = std::move(newEnemies);
                    enemyMeshesByPath.clear();
                    for (const EnemyPtr& p : enemies) {
                        const std::string& pathKey = p->modelPath;
                        if (enemyMeshesByPath.find(pathKey) == enemyMeshesByPath.end()) {
                            std::string path = "../assets/models/" + pathKey;
                            enemyMeshesByPath[pathKey] = loadMeshFromFile(path.c_str());
                        }
                    }
                    std::cout << "Enemies reloaded (" << enemies.size() << " enemies)" << std::endl;
                } else {
                    std::cout << "Enemy reload failed." << std::endl;
                }
            }
            reloadKeyPressedPrev = reloadPressed;
        }

        // ボート移動
        if (boatTransitionActive) {
            boatTransitionTimer += deltaTime;
            // ボード移動がどれくらい進んだかの割合
            float t = glm::min(1.0f, boatTransitionTimer / boatTransitionDuration);
            // smoothstep で滑らかに
            t = t * t * (3.0f - 2.0f * t);
            boatPos = boatTransitionStartBoat + (boatTransitionEnd - boatTransitionStartBoat) * t;
            // ボート位置から一番近い惑星の「上」方向でプレイヤーをボートの上に
            int nearestIdx = 0;
            float nearestD = glm::length(boatPos - planets[0].center);
            for (size_t i = 1; i < planets.size(); i++) {
                float d = glm::length(boatPos - planets[i].center);
                if (d < nearestD) { nearestD = d; nearestIdx = static_cast<int>(i); }
            }
            glm::vec3 boatUp = glm::normalize(boatPos - planets[nearestIdx].center);
            players[0].pos = boatPos + boatUp * playerHeightAboveBoat;
            // 到着処理
            if (t >= 1.0f && boatDestinationPlanetIndex >= 0) {
                players[0].planetIndex = boatDestinationPlanetIndex;
                boatPos = boatTransitionEnd;
                players[0].pos = boatTransitionEnd;
                players[0].onGround = true;
                players[0].velocity = glm::vec3(0.0f);
                restartPos = boatTransitionEnd;
                restartPlanetIndex = boatDestinationPlanetIndex;
                boatTransitionActive = false;
                if (bulletGhost) {
                    btTransform t;
                    t.setIdentity();
                    t.setOrigin(btVector3(players[0].pos.x, players[0].pos.y, players[0].pos.z));
                    bulletGhost->setWorldTransform(t);
                }
            }
        }
        if (!boatTransitionActive) {
            up = glm::normalize(players[0].pos - planets[players[0].planetIndex].center);
        }

        // 惑星2にいるときはBGMをboss.wavに切り替え（ゲームクリア後はBGMを流さない）
        if (!gameClear && players[0].planetIndex != currentBgmPlanetIndex) {
            currentBgmPlanetIndex = players[0].planetIndex;
            if (currentBgmPlanetIndex == 1 && bgmBoss) {
                Mix_PlayMusic(bgmBoss, -1);
            } else if (bgm) {
                Mix_PlayMusic(bgm, -1);
            }
        }

        float moveForward = 0.0f;
        float moveLeft = 0.0f;
        bool jumpPressed = false;
        bool attackPressed = false;
        bool dodgePressed = false;
        bool counterPressed = false;
        float dashSpeed = 1.0f;

        // Pキーで2P参加（1回だけ反応）
        bool pKeyNow = (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS);
        if (pKeyNow && !player2Joined) {
            player2Joined = true;
            players[1] = players[0];
            players[1].pos = planets[players[0].planetIndex].center + glm::normalize(players[0].pos - planets[players[0].planetIndex].center) * planets[players[0].planetIndex].radius;
            glm::vec3 fwdP, ignoreL;
            getForwardLeft(up, players[0].cameraYaw, fwdP, ignoreL);
            players[1].pos += fwdP * 2.0f;
            players[1].pos = planets[players[0].planetIndex].center + glm::normalize(players[1].pos - planets[players[0].planetIndex].center) * planets[players[0].planetIndex].radius;
            players[1].velocity = glm::vec3(0, 0, 0);
            players[1].onGround = true;
            players[1].attack = 10.0f;
            players[1].hp = 100.0f;
        }

        // ゲームパッド対応（1P）
        // 入力をSDLに取り込む
        SDL_PumpEvents();
        // コントローラーの状態更新
        SDL_GameControllerUpdate();
        if (sdlController && SDL_GameControllerGetAttached(sdlController)) {
            const float deadZone = 0.25f;
            // SDL_GameControllerGetAxisの範囲が32767までで、scaleをかけて1.0f以内に抑えるため
            const float scale = 1.0f / 32767.0f;
            // 左スティック
            float leftY = SDL_GameControllerGetAxis(sdlController, SDL_CONTROLLER_AXIS_LEFTY) * scale;
            float leftX = SDL_GameControllerGetAxis(sdlController, SDL_CONTROLLER_AXIS_LEFTX) * scale;
            // 触っていない時に動くのを防ぐ
            if (std::abs(leftY) < deadZone) leftY = 0.0f;
            if (std::abs(leftX) < deadZone) leftX = 0.0f;
            // 移動力に反映
            moveForward += leftY;
            moveLeft += leftX;

            // 右スティック
            float rightY = SDL_GameControllerGetAxis(sdlController, SDL_CONTROLLER_AXIS_RIGHTY) * scale;
            float rightX = SDL_GameControllerGetAxis(sdlController, SDL_CONTROLLER_AXIS_RIGHTX) * scale;
            if (std::abs(rightY) < deadZone) rightY = 0.0f;
            if (std::abs(rightX) < deadZone) rightX = 0.0f;
            players[0].cameraYaw += rightX * cameraSensitivity * deltaTime;
            players[0].cameraPitch -= rightY * cameraSensitivity * deltaTime;
            players[0].cameraPitch = glm::clamp(players[0].cameraPitch, -1.2f, -0.2f);
            // ジャンプ判定
            if (SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_A)) jumpPressed = true;
            // 攻撃判定（Xボタン）
            if (SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_X)) attackPressed = true;
            // 回避（Bボタン）
            if (SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_B)) dodgePressed = true;
            // カウンター（L1）
            if (SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_LEFTSHOULDER)) counterPressed = true;
            // ダッシュ判定
            if (SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) dashSpeed = 1.5f;
        }
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) counterPressed = true;
        glm::vec3 forward, left;
        getForwardLeft(up, players[0].cameraYaw, forward, left);

        // スティックを倒した方向を向く。移動ロック中は地上のみ向き固定、空中攻撃中は向き替え可
        if ((players[0].attackMoveLockRemaining <= 0.0f || !players[0].onGround) && (std::abs(moveForward) > 0.01f || std::abs(moveLeft) > 0.01f)) {
            glm::vec3 moveDir = forward * moveForward + left * moveLeft;
            float len = glm::length(moveDir);
            if (len > 0.001f) {
                // 移動方向を正規化
                moveDir /= len;
                players[0].facingYaw = getYawFromDirection(up, moveDir) + 3.14159265f;
            }
        }

        if (!boatTransitionActive) {
            if (transitionTimer <= 0.0f && !players[0].isDamaged && players[0].attackMoveLockRemaining <= 0.0f && dodgeTimer <= 0.0f) {
                glm::vec3 moveDelta = forward * moveForward * characterSpeed * deltaTime * dashSpeed
                    + left * moveLeft * characterSpeed * deltaTime * dashSpeed;
                glm::vec3 desiredPos = players[0].pos + moveDelta;
                // 壁当たり：球スイープで移動経路に障害があれば移動を打ち切り
                if (bulletOk && bulletWorld && bulletWallSphere && glm::length(moveDelta) > 1e-5f) {
                    glm::vec3 upForSweep = glm::normalize(players[0].pos - planets[players[0].planetIndex].center);
                    glm::vec3 sweepFrom = players[0].pos + upForSweep * 0.4f;  // 腰高で判定（地面に当たりにくくする）
                    glm::vec3 sweepTo = desiredPos + upForSweep * 0.4f;
                    btTransform fromBt, toBt;
                    fromBt.setIdentity();
                    fromBt.setOrigin(btVector3(sweepFrom.x, sweepFrom.y, sweepFrom.z));
                    toBt.setIdentity();
                    toBt.setOrigin(btVector3(sweepTo.x, sweepTo.y, sweepTo.z));
                    btVector3 sweepFromBt(sweepFrom.x, sweepFrom.y, sweepFrom.z);
                    btVector3 sweepToBt(sweepTo.x, sweepTo.y, sweepTo.z);
                    btCollisionWorld::ClosestConvexResultCallback sweepCallback(sweepFromBt, sweepToBt);
                    bulletWorld->convexSweepTest(bulletWallSphere, fromBt, toBt, sweepCallback);
                    if (sweepCallback.hasHit()) {
                        // 壁手前で一度止め、残りを壁に沿う方向（スライド）に投影して進める
                        float allowFrac = std::max(0.0f, sweepCallback.m_closestHitFraction - 0.02f);
                        glm::vec3 posAfterHit = players[0].pos + moveDelta * allowFrac;
                        glm::vec3 hitNormGlm(
                            sweepCallback.m_hitNormalWorld.x(),
                            sweepCallback.m_hitNormalWorld.y(),
                            sweepCallback.m_hitNormalWorld.z());
                        // 阻害された移動を壁面に投影 → 壁沿いのスライドベクトル
                        glm::vec3 blocked = moveDelta * (1.0f - allowFrac);
                        glm::vec3 slideVec = blocked - hitNormGlm * glm::dot(blocked, hitNormGlm);
                        const float slideEps = 1e-4f;
                        if (glm::length(slideVec) > slideEps) {
                            glm::vec3 slideFrom = posAfterHit + upForSweep * 0.4f;
                            glm::vec3 slideTo = slideFrom + slideVec;
                            btTransform fromBt2, toBt2;
                            fromBt2.setIdentity();
                            fromBt2.setOrigin(btVector3(slideFrom.x, slideFrom.y, slideFrom.z));
                            toBt2.setIdentity();
                            toBt2.setOrigin(btVector3(slideTo.x, slideTo.y, slideTo.z));
                            btVector3 sFrom(slideFrom.x, slideFrom.y, slideFrom.z);
                            btVector3 sTo(slideTo.x, slideTo.y, slideTo.z);
                            btCollisionWorld::ClosestConvexResultCallback slideCallback(sFrom, sTo);
                            bulletWorld->convexSweepTest(bulletWallSphere, fromBt2, toBt2, slideCallback);
                            float slideAllow = slideCallback.hasHit()
                                ? std::max(0.0f, slideCallback.m_closestHitFraction - 0.02f)
                                : 1.0f;
                            desiredPos = posAfterHit + slideVec * slideAllow;
                        } else {
                            desiredPos = posAfterHit;
                        }
                    }
                }
                players[0].pos = desiredPos;
                if (players[0].onGround && jumpPressed) {
                    players[0].velocity += up * 5.0f;
                    players[0].onGround = false;
                }
            }
            // Bボタン：向いている方向へ回避開始
            if (dodgePressed && !dodgePressedPrev && dodgeCooldown <= 0.0f && dodgeTimer <= 0.0f && players[0].attackDodgeLockRemaining <= 0.0f) {
                glm::vec3 dodgeFwd, dodgeLeftUnused;
                getForwardLeft(up, players[0].facingYaw, dodgeFwd, dodgeLeftUnused);
                dodgeDir = -dodgeFwd;
                dodgeTimer = dodgeDuration;
                dodgeCooldown = dodgeCooldownTime;
                dodgeStartHeight = glm::length(players[0].pos - planets[players[0].planetIndex].center);
                players[0].velocity = glm::vec3(0.0f);  // 回避後に通常落下するためリセット
            }
            if (dodgeTimer > 0.0f) {
                float dodgeSpeed = dodgeDistance / dodgeDuration;
                players[0].pos += dodgeDir * dodgeSpeed * deltaTime;
                glm::vec3 center = planets[players[0].planetIndex].center;
                // 空中回避：直前の高さ（惑星中心からの距離）を維持して浮遊
                float dist = glm::length(players[0].pos - center);
                if (dist > 1e-6f)
                    players[0].pos = center + (players[0].pos - center) / dist * dodgeStartHeight;
                dodgeTimer -= deltaTime;
            }
            if (dodgeCooldown > 0.0f) dodgeCooldown -= deltaTime;

            // 攻撃後＋0.5秒：攻撃時の高さを維持して浮遊（空中固定）
            if (attackHeightLockRemaining > 0.0f) {
                glm::vec3 center = planets[players[0].planetIndex].center;
                float dist = glm::length(players[0].pos - center);
                if (dist > 1e-6f)
                    players[0].pos = center + (players[0].pos - center) / dist * attackStartHeight;
            }
            // Bullet レイキャスト：足元にメッシュがあれば地形に沿わせ、穴の上なら重力で落ちる
            // 回避中・攻撃硬直中は浮遊のためスキップ。上昇中（ジャンプ直後）もスキップし、地上 or 落下中のみ判定
            glm::vec3 upForJump = glm::normalize(players[0].pos - planets[players[0].planetIndex].center);
            bool isRising = glm::dot(players[0].velocity, upForJump) > 0.5f;
            bool meshGround = false;
            if (dodgeTimer <= 0.0f && attackHeightLockRemaining <= 0.0f && bulletOk && bulletWorld && !isRising) {
                glm::vec3 center = planets[players[0].planetIndex].center;
                glm::vec3 upDir = glm::normalize(players[0].pos - center);
                glm::vec3 rayFrom3 = players[0].pos + upDir * 0.1f;
                glm::vec3 rayTo3 = players[0].pos - upDir * 0.1f;
                btVector3 rayFrom(rayFrom3.x, rayFrom3.y, rayFrom3.z);
                btVector3 rayTo(rayTo3.x, rayTo3.y, rayTo3.z);
                btCollisionWorld::ClosestRayResultCallback rayCallback(rayFrom, rayTo);
                bulletWorld->rayTest(rayFrom, rayTo, rayCallback);
                if (rayCallback.hasHit()) {
                    btVector3 hitPt = rayCallback.m_hitPointWorld;
                    glm::vec3 hitPos(hitPt.x(), hitPt.y(), hitPt.z());
                    float hitDist = glm::length(hitPos - center);
                    float playerDist = glm::length(players[0].pos - center);
                    if (playerDist - hitDist < 2.0f) {
                        players[0].pos = hitPos;
                        players[0].onGround = true;
                        players[0].velocity = glm::vec3(0.0f);
                        meshGround = true;
                    }
                }
                if (!meshGround && players[0].onGround) {
                    players[0].onGround = false;
                }
            }
            // 回避中・攻撃空中固定中は重力をかけず、終了後に通常通り落下
            if (dodgeTimer <= 0.0f && attackHeightLockRemaining <= 0.0f)
                updatePlayerPhysics(players[0], deltaTime, planets, &transitionTimer, bulletOk && bulletWorld);

            // 落下して惑星内部にめり込んだらリスタート地点へ
            if (bulletOk && bulletWorld) {
                float dist = glm::length(players[0].pos - planets[players[0].planetIndex].center);
                float r = planets[players[0].planetIndex].radius;
                if (dist < r * 0.5f) {
                    players[0].pos = restartPos;
                    players[0].planetIndex = restartPlanetIndex;
                    players[0].velocity = glm::vec3(0.0f);
                    players[0].onGround = true;
                }
            }
        }

        if (player2Joined) {
            float moveF2 = 0.0f, moveL2 = 0.0f;
            bool jump2 = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
            float dash2 = (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) ? 1.5f : 1.0f;
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveF2 -= 1.0f;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveF2 += 1.0f;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveL2 -= 1.0f;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveL2 += 1.0f;
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) players[1].cameraYaw -= cameraSensitivity * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) players[1].cameraYaw += cameraSensitivity * deltaTime;

            glm::vec3 up2 = glm::normalize(players[1].pos - planets[players[1].planetIndex].center);
            glm::vec3 fwd2, left2;
            getForwardLeft(up2, players[1].cameraYaw, fwd2, left2);
            if (std::abs(moveF2) > 0.01f || std::abs(moveL2) > 0.01f) {
                glm::vec3 moveDir2 = fwd2 * moveF2 + left2 * moveL2;
                float len2 = glm::length(moveDir2);
                if (len2 > 0.001f) {
                    moveDir2 /= len2;
                    players[1].facingYaw = getYawFromDirection(up2, moveDir2) + 3.14159265f;
                }
            }
            if (transitionTimer <= 0.0f) {
                players[1].pos += fwd2 * moveF2 * characterSpeed * deltaTime * dash2;
                players[1].pos += left2 * moveL2 * characterSpeed * deltaTime * dash2;
                if (players[1].onGround && jump2) {
                    players[1].velocity += up2 * 5.0f;
                    players[1].onGround = false;
                }
            }
            updatePlayerPhysics(players[1], deltaTime, planets, &transitionTimer);
        }

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || (sdlController && SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_BACK)))
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        float planetRadius = planets[players[0].planetIndex].radius;
        const float attackRangeMargin = 0.2f;
        for (EnemyPtr& ptr : enemies) {
            EnemyBase& e = *ptr;
            if (!e.alive) continue;
            // 追跡：どちらかのプレイヤーに向かう（1P基準で判定）
            float distToP0 = glm::length(players[0].pos - e.pos);
            glm::vec3 toPlayer = glm::normalize(players[0].pos - e.pos);
            if (distToP0 <= EnemyBase::Sensing && e.damageTimer <= 0.0f && !players[0].isDamaged && distToP0 >= e.getRadius()) {
                e.pos += toPlayer * e.speed * deltaTime;
                float r = planets[e.planetIndex].radius;
                e.pos = planets[e.planetIndex].center + glm::normalize(e.pos - planets[e.planetIndex].center) * r;
            }
            // 攻撃範囲：いずれかのプレイヤーが範囲内なら「範囲内」とする（タイマーは敵ごとに1つなので二重更新しない）
            float distToP1 = glm::length(players[1].pos - e.pos);
            bool inRangeOfP0 = (distToP0 <= e.getRadius() + attackRangeMargin);
            bool inRangeOfP1 = (distToP1 <= e.getRadius() + attackRangeMargin);
            bool inRangeOfAny = inRangeOfP0 || inRangeOfP1;
            if (!inRangeOfAny) {
                e.standByAttackTimer = -1.0f;  // 全員範囲外なら idle
            } else if (e.standByAttackTimer == -1.0f || e.standByAttackTimer == -2.0f) {
                if (!players[0].isDamagePrev && !players[1].isDamagePrev)
                    e.standByAttackTimer = 2.0f;  // 範囲内で idle/攻撃直後のときだけスタンバイ開始（1回だけ）
            }
            if (e.standByAttackTimer >= 0.0f) {
                float prevStandBy = e.standByAttackTimer;
                e.standByAttackTimer -= deltaTime;
                if (prevStandBy >= 0.5f && e.standByAttackTimer < 0.5f && attackPreSe) {
                    Mix_PlayChannel(-1, attackPreSe, 0);  // 敵の攻撃0.5秒前に鳴らす
                }
            }
            if (e.standByAttackTimer <= 0.0f && inRangeOfAny) {
                e.standByAttackTimer = -2.0f;  // 攻撃終了（次のフレームで範囲内なら再スタンバイ可能）
                if (inRangeOfP0 && !players[0].isDamagePrev) {
                    players[0].hp -= e.attack;
                    players[0].damageTimer = 1.0f;
                    players[0].isDamaged = true;
                    playerKnockbackFrom = e.pos;
                    if (players[0].hp <= 0) {
                        players[0].hp = 0;
                        players[0].pos = restartPos;
                        players[0].planetIndex = restartPlanetIndex;
                        players[0].velocity = glm::vec3(0.0f);
                        players[0].onGround = true;
                    }
                }
                if (inRangeOfP1 && !players[1].isDamagePrev) {
                    players[1].hp -= e.attack;
                    players[1].damageTimer = 1.0f;
                    players[1].isDamaged = true;
                    playerKnockbackFrom = e.pos;
                    if (players[1].hp <= 0) {
                        players[1].hp = 0;
                        players[1].pos = restartPos;
                        players[1].planetIndex = restartPlanetIndex;
                        players[1].velocity = glm::vec3(0.0f);
                        players[1].onGround = true;
                    }
                }
            }
        }

        // Xボタン攻撃：正面にいる敵のうち最初の1体にダメージ（1Pのみ）。当たらなくても硬直・高さ維持する
        const float attackRange = 1.8f;
        const float attackAngle = 0.8f;
        if (attackPressed && !attackPressedPrev && players[0].attackCooldownRemaining <= 0.0f) {
            attackStartHeight = glm::length(players[0].pos - planets[players[0].planetIndex].center);
            players[0].velocity = glm::vec3(0.0f);
            bool hitTarget = false;
            glm::vec3 attackFwd, attackLeftUnused;
            getForwardLeft(up, players[0].facingYaw, attackFwd, attackLeftUnused);
            for (EnemyPtr& ptr : enemies) {
                EnemyBase& e = *ptr;
                if (!e.alive) continue;
                glm::vec3 toEnemy = glm::normalize(e.pos - players[0].pos);
                float dot = glm::dot(-attackFwd, toEnemy);
                float dist = glm::length(e.pos - players[0].pos);
                float effectiveRange = attackRange + e.getRadius();
                if (dist <= effectiveRange && dot >= attackAngle) {
                    e.hp -= players[0].attack;
                    if (e.hp <= 0) e.damageTimer = 1.0f;  // 死ぬ時だけノックバック
                    if (attackSe) Mix_PlayChannel(-1, attackSe, 0);
                    players[0].attackCooldownRemaining = 0.3f;
                    players[0].attackIndex++;
                    bool wasThirdHit = false;
                    if (players[0].attackIndex == 3) {
                        wasThirdHit = true;
                        players[0].attackCooldownRemaining = 1.5f;
                        players[0].attackIndex = 0;
                    }
                    players[0].attackMoveLockRemaining = std::min(players[0].attackCooldownRemaining, 1.0f) + 0.5f;
                    players[0].attackDodgeLockRemaining = std::max(0.0f, players[0].attackMoveLockRemaining - 0.5f);
                    if (wasThirdHit && !players[0].onGround)
                        attackHeightLockRemaining = 0.5f;  // 空中で最後の攻撃は0.5秒だけ浮遊してから落下
                    else
                        attackHeightLockRemaining = players[0].attackMoveLockRemaining + 0.5f;
                    hitTarget = true;
                    if (e.hp <= 0) {
                        e.hp = 0;
                    };
                    break;
                }
            }
            if (!hitTarget) {
                players[0].attackCooldownRemaining = 0.3f;
                players[0].attackIndex++;
                bool wasThirdHit = false;
                if (players[0].attackIndex == 3) {
                    wasThirdHit = true;
                    players[0].attackCooldownRemaining = 1.5f;
                    players[0].attackIndex = 0;
                }
                players[0].attackMoveLockRemaining = std::min(players[0].attackCooldownRemaining, 1.0f) + 0.5f;
                players[0].attackDodgeLockRemaining = std::max(0.0f, players[0].attackMoveLockRemaining - 0.5f);
                if (wasThirdHit && !players[0].onGround)
                    attackHeightLockRemaining = 0.7f;  // 空中で最後の攻撃は0.5秒だけ浮遊してから落下
                else
                    attackHeightLockRemaining = players[0].attackMoveLockRemaining + 0.2f;
                if (attackMissSe) Mix_PlayChannel(-1, attackMissSe, 0);
            }
        }

        // L（L1）カウンター：敵のstandByAttackTimerが0.2f以下の時に押すと成功、counter.wav・ノックバック・攻撃力2倍ダメージ
        if (counterPressed && !counterKeyPressedPrev && !boatTransitionActive) {
            glm::vec3 counterFwd, counterLeftUnused;
            getForwardLeft(up, players[0].facingYaw, counterFwd, counterLeftUnused);
            for (EnemyPtr& ptr : enemies) {
                EnemyBase& e = *ptr;
                if (!e.alive) continue;
                if (e.standByAttackTimer <= 0.0f || e.standByAttackTimer > 0.2f) continue;
                glm::vec3 toEnemy = glm::normalize(e.pos - players[0].pos);
                float dot = glm::dot(-counterFwd, toEnemy);
                float dist = glm::length(e.pos - players[0].pos);
                float effectiveRange = attackRange + e.getRadius();
                if (dist <= effectiveRange && dot >= attackAngle) {
                    if (counterSe) Mix_PlayChannel(-1, counterSe, 0);
                    e.standByAttackTimer = -1.0f;  // 敵の攻撃をキャンセル
                    e.damageTimer = 1.0f;         // ノックバック
                    e.hp -= 2.0f * players[0].attack;  // 攻撃力2倍ダメージ
                    if (e.hp <= 0) e.hp = 0;
                    break;
                }
            }
        }

        if (players[0].attackCooldownRemaining >= 0.0f) {
            players[0].attackCooldownRemaining -= deltaTime;
        }
        if (players[0].attackMoveLockRemaining > 0.0f) {
            players[0].attackMoveLockRemaining -= deltaTime;
            if (players[0].attackMoveLockRemaining < 0.0f) players[0].attackMoveLockRemaining = 0.0f;
            if (players[0].attackMoveLockRemaining <= 0.0f) players[0].attackIndex = 0;  // 歩けるようになったらコンボリセット
        }
        if (players[0].attackDodgeLockRemaining > 0.0f) {
            players[0].attackDodgeLockRemaining -= deltaTime;
            if (players[0].attackDodgeLockRemaining < 0.0f) players[0].attackDodgeLockRemaining = 0.0f;
        }
        if (attackHeightLockRemaining > 0.0f) {
            attackHeightLockRemaining -= deltaTime;
            if (attackHeightLockRemaining < 0.0f) attackHeightLockRemaining = 0.0f;
        }

        for (EnemyPtr& ptr : enemies) {
            EnemyBase& e = *ptr;
            if (e.damageTimer > 0.0f) {
                glm::vec3 toEnemy = glm::normalize(e.pos - players[0].pos);
                e.pos += toEnemy * deltaTime;
                e.pos = planets[e.planetIndex].center + glm::normalize(e.pos - planets[e.planetIndex].center) * planets[e.planetIndex].radius;
                e.damageTimer -= deltaTime;
            } else {
                if (e.hp <= 0) {
                    e.hp = 0;
                    e.alive = false;
                    // 現在の惑星で倒した敵の位置を記録（鍵出現位置に使う）
                    if (e.planetIndex == players[0].planetIndex)
                        lastDefeatedEnemyPos = e.pos;
                    // ボス撃破でスター出現（撃破前のボスがいた場所に置く）
                    if (e.isBoss() && !starVisibleFromBoss &&
                            e.planetIndex >= 0 && static_cast<size_t>(e.planetIndex) < planets.size()) {
                        starVisibleFromBoss = true;
                        starBossPlanetIndex = e.planetIndex;
                        starPos = e.pos;  // ボスがいた場所
                    }
                }
            }
        }

        if (players[0].damageTimer > 0.0f) {
            glm::vec3 toPlayer = glm::normalize(players[0].pos - playerKnockbackFrom);
            players[0].pos += toPlayer * deltaTime;
            players[0].damageTimer -= deltaTime;
        } else {
            players[0].isDamaged = false;
        }

        // 今いる惑星の敵を全て倒したら鍵を出現させる
        std::vector<EnemyBase*> currentPlanetEnemies;
        for (const EnemyPtr& p : enemies) {
            if (p->planetIndex == players[0].planetIndex)
                currentPlanetEnemies.push_back(p.get());
        }
        bool allEnemiesDead = true;
        for (EnemyBase* e : currentPlanetEnemies) {
            if (e->alive) { allEnemiesDead = false; break; }
        }
        if (allEnemiesDead && !keyVisible && !keyObtained) {
            keyVisible = true;
            keyPlanetIndex = players[0].planetIndex;
            keyPos = lastDefeatedEnemyPos;  // 最後に倒した敵の場所に鍵を出す
        }

        // 鍵に触れたら取得して消す＆ボートを出現させる
        if (keyVisible) {
            float distToKey = glm::length(players[0].pos - keyPos);
            if (distToKey < keyPickupRadius) {
                keyVisible = false;
                keyObtained = true;
                int curIdx = players[0].planetIndex;
                boatSpawnPlanetIndex = curIdx;
                // 現在惑星から一番近い別惑星を到着先に
                float nearestDist = 1e30f;
                for (size_t i = 0; i < planets.size(); i++) {
                    if (static_cast<int>(i) == curIdx) continue;
                    float d = glm::length(planets[i].center - planets[curIdx].center);
                    if (d < nearestDist) {
                        nearestDist = d;
                        boatDestinationPlanetIndex = static_cast<int>(i);
                    }
                }
                // ボートを現在惑星の表面近くに配置
                float boatHeight = planets[curIdx].radius - 0.15f;
                boatPos = planets[curIdx].center + glm::normalize(glm::vec3(0.0f, -1.0f, 0.5f)) * boatHeight;
            }
        }

        // ボートに触れたら到着先へ移動開始（ボートが出現した惑星にいる時のみ）
        if (!boatTransitionActive && keyObtained && boatSpawnPlanetIndex >= 0 && boatDestinationPlanetIndex >= 0
            && players[0].planetIndex == boatSpawnPlanetIndex) {
            float distToBoat = glm::length(players[0].pos - boatPos);
            if (distToBoat < boatTouchRadius) {
                boatTransitionActive = true;
                boatTransitionTimer = 0.0f;
                boatTransitionStartBoat = boatPos;
                const Planet& dest = planets[boatDestinationPlanetIndex];
                const Planet& from = planets[boatSpawnPlanetIndex];
                glm::vec3 toDest = glm::normalize(dest.center - from.center);
                boatTransitionEnd = dest.center - toDest * dest.radius;
            }
        }

        // スターに触れたらゲームクリア（ボス撃破で出現したスターが存在するときだけ判定）
        bool starExists = (starVisibleFromBoss && players[0].planetIndex == starBossPlanetIndex);
        if (!gameClear && starExists) {
            float distToStar = glm::length(players[0].pos - starPos);
            if (distToStar < starTouchRadius) {
                gameClear = true;
                Mix_HaltMusic();
                if (clearSe) Mix_PlayChannel(-1, clearSe, 0);
                std::cout << "Game Clear!" << std::endl;
            }
        }

        glm::mat4 view = getPlayerView(players[0], cameraDistance, planets);
        glm::mat4 view2P = player2Joined ? getPlayerView(players[1], cameraDistance, planets) : view;

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // これから描画するときにどのプログラムを使うのか設定
        glUseProgram(shaderProgram);

        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

        auto drawScene = [&](const glm::mat4& viewMat, const glm::mat4& projMat) {
            // CPU側のMVPをシェーダーのそれぞれのuniformに渡して使えるようにしている
            // 惑星描画
            glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(viewMat));
            glUniformMatrix4fv(locProj, 1, GL_FALSE, glm::value_ptr(projMat));

        // 惑星描画
        for (size_t i = 0; i < planets.size(); i++) {
            glm::mat4 planetModel = glm::translate(glm::mat4(1.0f), planets[i].center) * glm::scale(glm::mat4(1.0f), glm::vec3(planets[i].radius));
            glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(planetModel));
            glUniform3f(locObjectColor, planets[i].color.x, planets[i].color.y, planets[i].color.z);
            auto it = planetMeshesByPath.find(planets[i].modelPath);
            if (it != planetMeshesByPath.end() && !it->second.empty()) {
                for (const LoadedMesh& m : it->second) {
                    glBindVertexArray(m.VAO);
                    if (m.textureID != 0) {
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, m.textureID);
                        glUniform1i(locDiffuseTexture, 0);
                        glUniform1i(locUseTexture, 1);
                    } else {
                        glUniform1i(locUseTexture, 0);
                    }
                    glDrawElements(GL_TRIANGLES, m.indexCount, GL_UNSIGNED_INT, 0);
                }
                glUniform1i(locUseTexture, 0);
            } else {
                glBindVertexArray(sphereVAO);
                glUniform1i(locUseTexture, 0);
                glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
            }
        }

        const float playerScale = 0.25f;
        auto drawCharacter = [&](const glm::vec3& pos, float scale, const glm::vec3& fallbackColor,
                const glm::vec3& up, float yaw, const std::vector<LoadedMesh>& meshes,
                const glm::vec3* colorOverride = nullptr) {
            glm::vec3 fwd, left;
            getForwardLeft(up, yaw, fwd, left);
            glm::vec3 right = -left;
            glm::mat4 orient = glm::mat4(1.0f);
            orient[0] = glm::vec4(-fwd, 0.0f);
            orient[1] = glm::vec4(up, 0.0f);
            orient[2] = glm::vec4(right, 0.0f);
            orient[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            glm::mat4 model = glm::translate(glm::mat4(1.0f), pos)
                * orient
                * glm::scale(glm::mat4(1.0f), glm::vec3(scale));
            if (!meshes.empty()) {
                glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
                for (const LoadedMesh& m : meshes) {
                    glBindVertexArray(m.VAO);
                    if (m.textureID != 0) {
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, m.textureID);
                        glUniform1i(locDiffuseTexture, 0);
                        glUniform1i(locUseTexture, 1);
                    } else {
                        glUniform1i(locUseTexture, 0);
                    }
                    if (colorOverride) {
                        glUniform3f(locObjectColor, colorOverride->x, colorOverride->y, colorOverride->z);
                    } else {
                        glUniform3f(locObjectColor, m.diffuseColor[0], m.diffuseColor[1], m.diffuseColor[2]);
                    }
                    glDrawElements(GL_TRIANGLES, m.indexCount, GL_UNSIGNED_INT, 0);
                }
                glUniform1i(locUseTexture, 0);
            } else {
                glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
                glBindVertexArray(VAO);
                glm::vec3 c = colorOverride ? *colorOverride : fallbackColor;
                glUniform3f(locObjectColor, c.x, c.y, c.z);
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
        };
        glm::vec3 up0 = glm::normalize(players[0].pos - planets[players[0].planetIndex].center);
        drawCharacter(players[0].pos, playerScale, glm::vec3(0.0f, 0.0f, 1.0f), up0, players[0].facingYaw, playerMeshes);
        if (player2Joined) {
            glm::vec3 up1 = glm::normalize(players[1].pos - planets[players[1].planetIndex].center);
            drawCharacter(players[1].pos, playerScale, glm::vec3(1.0f, 0.5f, 0.0f), up1, players[1].facingYaw, playerMeshes);
        }

        // 敵描画（惑星に立ててプレイヤー方向を向く、modelPath と drawScale はデータ駆動）+ 頭上にID
        for (size_t ei = 0; ei < enemies.size(); ei++) {
            const EnemyBase& e = *enemies[ei];
            if (!e.alive) continue;
            auto eit = enemyMeshesByPath.find(e.modelPath);
            if (eit == enemyMeshesByPath.end() || eit->second.empty())
                eit = enemyMeshesByPath.find("enemy.obj");
            if (eit == enemyMeshesByPath.end() || eit->second.empty())
                continue;
            glm::vec3 enemyUp = glm::normalize(e.pos - planets[e.planetIndex].center);
            glm::vec3 toPlayer = glm::normalize(players[0].pos - e.pos);
            float enemyFacingYaw = getYawFromDirection(enemyUp, toPlayer) + 3.14159265f;
            drawCharacter(e.pos, e.drawScale, glm::vec3(0.0f, 1.0f, 0.0f), enemyUp, enemyFacingYaw, eit->second);
            // 敵の頭上にID（1始まり）をビルボード表示
            if (font) {
                auto [texId, texSize] = getTextTexture(std::to_string(ei + 1));
                if (texId != 0 && texSize.x > 0 && texSize.y > 0) {
                    glm::vec3 camPos(glm::inverse(viewMat)[3]);
                    glm::vec3 quadCenter = e.pos + enemyUp * 0.8f;
                    glm::vec3 forward = glm::normalize(camPos - quadCenter);
                    glm::vec3 right = glm::normalize(glm::cross(enemyUp, forward));
                    if (glm::length(right) < 0.01f) right = glm::normalize(glm::cross(enemyUp, glm::vec3(0, 0, 1)));
                    glm::vec3 upQuad = glm::cross(forward, right);
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
                    glBindVertexArray(textQuadVAO);
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                    glUniform1i(locUseTexture, 0);
                    glDepthMask(GL_TRUE);
                    glDisable(GL_BLEND);
                }
            }
        }

        // 鍵描画（出現した惑星上で表示）
        if (keyVisible && keyPlanetIndex >= 0 && static_cast<size_t>(keyPlanetIndex) < planets.size()) {
            glm::vec3 keyUp = glm::normalize(keyPos - planets[keyPlanetIndex].center);
            drawCharacter(keyPos, keyScale, keyColor, keyUp, 0.0f, keyMeshes, &keyColor);
        }

        // ボート描画（ボートが出現した惑星にいる時か移動中のみ表示）
        if (keyObtained && boatSpawnPlanetIndex >= 0 &&
            (players[0].planetIndex == boatSpawnPlanetIndex || boatTransitionActive)) {
            int drawPlanet = boatTransitionActive ? 0 : boatSpawnPlanetIndex;
            if (boatTransitionActive) {
                float nearestD = glm::length(boatPos - planets[0].center);
                for (size_t i = 1; i < planets.size(); i++) {
                    float d = glm::length(boatPos - planets[i].center);
                    if (d < nearestD) { nearestD = d; drawPlanet = static_cast<int>(i); }
                }
            }
            glm::vec3 boatUp = glm::normalize(boatPos - planets[drawPlanet].center);
            drawCharacter(boatPos, boatScale, glm::vec3(0.4f, 0.25f, 0.1f), boatUp, 0.0f, boatMeshes);
        }

        // スター描画（ボス撃破後のみ存在・描画）
        if (!gameClear) {
            int starPlanetIdx = -1;
            if (starVisibleFromBoss && starBossPlanetIndex >= 0 && static_cast<size_t>(starBossPlanetIndex) < planets.size())
                starPlanetIdx = starBossPlanetIndex;
            if (starPlanetIdx >= 0) {
                glm::vec3 starUp = glm::normalize(starPos - planets[starPlanetIdx].center);
                glm::vec3 starColor(1.0f, 0.9f, 0.2f);
                drawCharacter(starPos, starScale, starColor, starUp, 0.0f, starMeshes);
            }
        }
        };

        if (!player2Joined) {
            glViewport(0, 0, fbWidth, fbHeight);
            float aspect = static_cast<float>(fbWidth) / static_cast<float>(fbHeight);
            glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
            drawScene(view, proj);
        } else {
            float halfW = fbWidth * 0.5f;
            float aspectHalf = halfW / static_cast<float>(fbHeight);
            glm::mat4 projHalf = glm::perspective(glm::radians(45.0f), aspectHalf, 0.1f, 100.0f);
            glViewport(0, 0, static_cast<GLsizei>(halfW), fbHeight);
            drawScene(view, projHalf);
            glViewport(static_cast<GLsizei>(halfW), 0, static_cast<GLsizei>(halfW), fbHeight);
            drawScene(view2P, projHalf);
        }

        attackPressedPrev = attackPressed;
        dodgePressedPrev = dodgePressed;
        counterKeyPressedPrev = counterPressed;
        players[0].isDamagePrev = players[0].isDamaged;

        // バッファーを入れ替える
        glfwSwapBuffers(window);
        // std::cout << currentPlanetIndex << std::endl;
    }
		
	// ゲーム終了処理
    if (sdlController) {
        SDL_GameControllerClose(sdlController);
    }
    for (auto& p : textTextureCache)
        glDeleteTextures(1, &p.second.first);
    if (font) TTF_CloseFont(font);
    TTF_Quit();
    glDeleteVertexArrays(1, &textQuadVAO);
    glDeleteBuffers(1, &textQuadVBO);
    SDL_Quit();

    // 再生中の曲を止める（オーディオは開いたままだから他の曲を流せる）
    Mix_HaltMusic();
    // 曲データを解放する
    if (bgm) Mix_FreeMusic(bgm);
    if (bgmBoss) Mix_FreeMusic(bgmBoss);
    if (attackSe) Mix_FreeChunk(attackSe);
    if (attackMissSe) Mix_FreeChunk(attackMissSe);
    if (attackPreSe) Mix_FreeChunk(attackPreSe);
    if (counterSe) Mix_FreeChunk(counterSe);
    if (clearSe) Mix_FreeChunk(clearSe);
    // オーディオを閉じる（曲を流せなくなる）
    Mix_CloseAudio();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    if (bulletWorld) {
        if (bulletCharController) {
            bulletWorld->removeAction(bulletCharController);
            delete bulletCharController;
            bulletCharController = nullptr;
        }
        if (bulletGhost) {
            bulletWorld->removeCollisionObject(bulletGhost);
            delete bulletGhost;
            bulletGhost = nullptr;
        }
        if (bulletCapsule) {
            delete bulletCapsule;
            bulletCapsule = nullptr;
        }
        if (bulletWallSphere) {
            delete bulletWallSphere;
            bulletWallSphere = nullptr;
        }
        for (btRigidBody* b : bulletPlanetBodies) {
            if (b) {
                bulletWorld->removeRigidBody(b);
                delete b;
            }
        }
        for (btBvhTriangleMeshShape* s : bulletPlanetShapes) { if (s) delete s; }
        for (btTriangleMesh* m : bulletPlanetMeshes) { if (m) delete m; }
        delete bulletWorld;
        bulletWorld = nullptr;
        delete bulletSolver;
        delete bulletBroadphase;
        delete bulletDispatcher;
        delete bulletCollisionConfig;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}