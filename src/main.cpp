// 学習用にコメントをつけています。
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL_mixer.h>
#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include "./shader.h"
#include "./model_loader.h"
#include "./planet.h"
#include "./player.h"
#include "./enemy.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

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

    // 惑星モデルをロードする
    std::vector<LoadedMesh> planetMeshes = loadMeshFromFile("../assets/models/planet.obj");
    // プレイヤーモデルをロードする
    std::vector<LoadedMesh> playerMeshes = loadMeshFromFile("../assets/models/player.obj");
    // 敵モデルをロードする
    std::vector<LoadedMesh> enemyMeshes = loadMeshFromFile("../assets/models/enemy.obj");
    // 鍵モデルをロードする（敵全員撃破で出現）
    std::vector<LoadedMesh> keyMeshes = loadMeshFromFile("../assets/models/key.obj");
    // ボートモデル（鍵取得で惑星近くに出現）
    std::vector<LoadedMesh> boatMeshes = loadMeshFromFile("../assets/models/boat.obj");
    // スター（惑星2に配置、触れるとゲームクリア）
    std::vector<LoadedMesh> starMeshes = loadMeshFromFile("../assets/models/star.obj");

    // シェーダープログラム内のMVPのIDを探して格納
    GLint locModel = glGetUniformLocation(shaderProgram, "model");
    GLint locView = glGetUniformLocation(shaderProgram, "view");
    GLint locProj = glGetUniformLocation(shaderProgram, "projection");
    GLint locObjectColor = glGetUniformLocation(shaderProgram, "objectColor");
    GLint locUseTexture = glGetUniformLocation(shaderProgram, "useTexture");
    GLint locDiffuseTexture = glGetUniformLocation(shaderProgram, "diffuseTexture");

    // 惑星の初期化
    std::vector<Planet> planets = {
        { glm::vec3(0.0f, 0.0f, 0.0f), 8.0f, glm::vec3(1.0f, 0.0f, 0.0f) },
        { glm::vec3(32.0f, 0.0f, 0.0f), 10.0f, glm::vec3(0.0f, 1.0f, 0.0f) },
    };

    const float characterSpeed = 3.5f;
    const float cameraDistance = 12.0f;
    const float cameraSensitivity = 2.5f;

    // 1P/2P 状態（players[0]=1P, players[1]=2P）
    PlayerState players[2];
    players[0].pos = glm::vec3(0.0f, planets[0].radius, 0.0f);
    players[0].planetIndex = 0;
    players[0].cameraPitch = 0.4f;
    players[0].attack = 10.0f;
    players[0].hp = 100.0f;
    glm::vec3 startPos = players[0].pos;
    bool player2Joined = false;

    // 敵5体を惑星上に散らす
    std::vector<EnemyState> enemies(1);
    {
        glm::vec3 center = planets[0].center;
        float radius = planets[0].radius;
        for (size_t i = 0; i < enemies.size(); i++) {
            float t = static_cast<float>(i) / 5.0f * 6.28318531f;
            float u = 0.3f * std::sin(static_cast<float>(i) * 1.7f);
            enemies[i].pos = center + radius * glm::normalize(glm::vec3(std::cos(t), std::sin(t), u));
            enemies[i].planetIndex = 0;
        }
    }
    glm::vec3 playerKnockbackFrom(0.0f);

    // 鍵（敵全員撃破で出現）
    bool keyVisible = false;
    bool keyObtained = false;
    glm::vec3 keyPos(0.0f);
    const float keyScale = 2.0f;
    const float keyPickupRadius = 1.2f;
    const glm::vec3 keyColor(0.85f, 0.65f, 0.13f);  // 金色

    // ボート（鍵取得で惑星近くに出現）
    glm::vec3 boatPos(0.0f);
    const float boatScale = 0.8f;
    const float boatTouchRadius = 1.8f;
    const float playerHeightAboveBoat = 0.7f;

    // ボートで次の惑星へ移動（触れたら少しずつ移動して到着）
    bool boatTransitionActive = false;
    float boatTransitionTimer = 0.0f;
    const float boatTransitionDuration = 5.0f;
    glm::vec3 boatTransitionStartBoat, boatTransitionEnd;

    // スター（惑星2に配置、触れるとゲームクリア）
    glm::vec3 starPos = planets[1].center + glm::normalize(glm::vec3(0.0f, 1.0f, 0.6f)) * (planets[1].radius + 0.2f);
    const float starScale = 0.3f;
    const float starTouchRadius = 1.5f;
    bool gameClear = false;

    // 時間情報
    double lastTime = glfwGetTime();
    float transitionTimer = 0.0f;
    float damageTimer = 0.0f;
    bool attackPressedPrev = false;
    bool dodgePressedPrev = false;
    float dodgeCooldown = 0.0f;
    float dodgeTimer = 0.0f;
    glm::vec3 dodgeDir(0.0f);
    const float dodgeDistance = 1.2f;
    const float dodgeDuration = 0.5f;
    const float dodgeCooldownTime = 1.0f;

    // 深度テストをONにして奥行きに応じて描画できるようにする（描画順ではなく、手前にあるものが上書きされて描画される）
    glEnable(GL_DEPTH_TEST);
		
	// ゲームループ
    while (!glfwWindowShouldClose(window)) {
        // 入力などのイベントを処理する
        glfwPollEvents();
        double currentTime = glfwGetTime();
        // 前フレームからの経過時間を計算
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        glm::vec3 up = glm::normalize(players[0].pos - planets[players[0].planetIndex].center);

        // ボート移動
        if (boatTransitionActive) {
            boatTransitionTimer += deltaTime;
            // ボード移動がどれくらい進んだかの割合
            float t = glm::min(1.0f, boatTransitionTimer / boatTransitionDuration);
            // smoothstep で滑らかに
            t = t * t * (3.0f - 2.0f * t);
            // スタート地点から到着点へのベクトルに割合をかけて進む
            boatPos = boatTransitionStartBoat + (boatTransitionEnd - boatTransitionStartBoat) * t;
            // 現在のボート位置から一番近い惑星の「上」方向で、プレイヤーをボートの上に置く
            float d0 = glm::length(boatPos - planets[0].center);
            float d1 = glm::length(boatPos - planets[1].center);
            glm::vec3 boatUp = (d0 <= d1)
                ? glm::normalize(boatPos - planets[0].center)
                : glm::normalize(boatPos - planets[1].center);
            players[0].pos = boatPos + boatUp * playerHeightAboveBoat;
            // 到着処理
            if (t >= 1.0f) {
                players[0].planetIndex = 1;
                boatPos = boatTransitionEnd;
                players[0].pos = boatTransitionEnd;
                players[0].onGround = true;
                players[0].velocity = glm::vec3(0.0f);
                boatTransitionActive = false;
            }
        }
        if (!boatTransitionActive) {
            up = glm::normalize(players[0].pos - planets[players[0].planetIndex].center);
        }

        float moveForward = 0.0f;
        float moveLeft = 0.0f;
        bool jumpPressed = false;
        bool attackPressed = false;
        bool dodgePressed = false;
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
            // ダッシュ判定
            if (SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) dashSpeed = 1.5f;
        }
        glm::vec3 forward, left;
        getForwardLeft(up, players[0].cameraYaw, forward, left);

        // スティックを倒した方向を向く（描画の正面を移動方向に）
        if (std::abs(moveForward) > 0.01f || std::abs(moveLeft) > 0.01f) {
            glm::vec3 moveDir = forward * moveForward + left * moveLeft;
            float len = glm::length(moveDir);
            if (len > 0.001f) {
                // 移動方向を正規化
                moveDir /= len;
                players[0].facingYaw = getYawFromDirection(up, moveDir) + 3.14159265f;
            }
        }

        if (!boatTransitionActive) {
            if (transitionTimer <= 0.0f && !players[0].isDamaged) {
                players[0].pos += forward * moveForward * characterSpeed * deltaTime * dashSpeed;
                players[0].pos += left * moveLeft * characterSpeed * deltaTime * dashSpeed;
                if (players[0].onGround && jumpPressed) {
                    players[0].velocity += up * 5.0f;
                    players[0].onGround = false;
                }
            }
            // Bボタン：向いている方向へ回避開始（攻撃を受けた時のように少しずつ移動）
            if (dodgePressed && !dodgePressedPrev && players[0].onGround && dodgeCooldown <= 0.0f && dodgeTimer <= 0.0f) {
                glm::vec3 dodgeFwd, dodgeLeftUnused;
                getForwardLeft(up, players[0].facingYaw, dodgeFwd, dodgeLeftUnused);
                dodgeDir = -dodgeFwd;
                dodgeTimer = dodgeDuration;
                dodgeCooldown = dodgeCooldownTime;
            }
            if (dodgeTimer > 0.0f) {
                float dodgeSpeed = dodgeDistance / dodgeDuration;
                players[0].pos += dodgeDir * dodgeSpeed * deltaTime;
                glm::vec3 center = planets[players[0].planetIndex].center;
                float radius = planets[players[0].planetIndex].radius;
                players[0].pos = center + glm::normalize(players[0].pos - center) * radius;
                dodgeTimer -= deltaTime;
            }
            if (dodgeCooldown > 0.0f) dodgeCooldown -= deltaTime;
            updatePlayerPhysics(players[0], deltaTime, planets, &transitionTimer);
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
        for (size_t pi = 0; pi < 2; pi++) {
            for (EnemyState& e : enemies) {
                if (!e.alive) continue;
                float distToEnemy = glm::length(players[pi].pos - e.pos);
                glm::vec3 toPlayer = glm::normalize(players[pi].pos - e.pos);
                if (distToEnemy <= EnemyState::Sensing && e.damageTimer <= 0.0f && !players[0].isDamaged) {
                    e.pos += toPlayer * EnemyState::Speed * deltaTime;
                    float r = planets[e.planetIndex].radius;
                    e.pos = planets[e.planetIndex].center + glm::normalize(e.pos - planets[e.planetIndex].center) * r;
                }
                if (players[pi].onGround && distToEnemy <= EnemyState::Radius && !players[pi].isDamagePrev) {
                    players[pi].hp -= EnemyState::Attack;
                    players[pi].damageTimer = 1.0f;
                    players[pi].isDamaged = true;
                    playerKnockbackFrom = e.pos;
                    if (players[pi].hp <= 0) {
                        players[pi].hp = 0;
                        players[pi].pos = startPos;
                    }
                }
            }
        }

        // Xボタン攻撃：正面にいる敵のうち最初の1体にダメージ（1Pのみ）
        const float attackRange = 1.8f;
        const float attackAngle = 0.8f;
        if (attackPressed && !attackPressedPrev) {
            glm::vec3 attackFwd, attackLeftUnused;
            getForwardLeft(up, players[0].facingYaw, attackFwd, attackLeftUnused);
            for (EnemyState& e : enemies) {
                if (!e.alive) continue;
                glm::vec3 toEnemy = glm::normalize(e.pos - players[0].pos);
                float dot = glm::dot(-attackFwd, toEnemy);
                float dist = glm::length(e.pos - players[0].pos);
                if (dist <= attackRange && dot >= attackAngle) {
                    e.hp -= players[0].attack;
                    e.damageTimer = 1.0f;
                    break;
                }
            }
        }

        for (EnemyState& e : enemies) {
            if (e.damageTimer > 0.0f) {
                glm::vec3 toEnemy = glm::normalize(e.pos - players[0].pos);
                e.pos += toEnemy * deltaTime;
                e.pos = glm::normalize(e.pos - planets[e.planetIndex].center) * planets[e.planetIndex].radius + planets[e.planetIndex].center;
                e.damageTimer -= deltaTime;
            }else {
                if (e.hp <= 0) {
                    e.hp = 0;
                    e.alive = false;
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

        // 敵を全て倒したら鍵を出現させる
        bool allEnemiesDead = true;
        for (const EnemyState& e : enemies) { if (e.alive) { allEnemiesDead = false; break; } }
        if (allEnemiesDead && !keyVisible && !keyObtained) {
            keyVisible = true;
            // 惑星表面より少し上に配置（埋まらないようにオフセット）
            float keyHeight = planets[0].radius + 1.0f;
            keyPos = planets[0].center + glm::normalize(glm::vec3(0.0f, 1.0f, 0.4f)) * keyHeight;
        }

        // 鍵に触れたら取得して消す＆ボートを出現させる
        if (keyVisible) {
            float distToKey = glm::length(players[0].pos - keyPos);
            if (distToKey < keyPickupRadius) {
                keyVisible = false;
                keyObtained = true;
                // ボートを惑星表面の近くに配置（鍵と反対側あたり）
                float boatHeight = planets[0].radius - 0.15f;
                boatPos = planets[0].center + glm::normalize(glm::vec3(0.0f, -1.0f, 0.5f)) * boatHeight;
            }
        }

        // ボートに触れたら次の惑星へ一緒に移動開始（惑星0にいる時のみ・到着後は発動しない）fa
        if (!boatTransitionActive && players[0].planetIndex == 0) {
            float distToBoat = glm::length(players[0].pos - boatPos);
            if (distToBoat < boatTouchRadius) {
                boatTransitionActive = true;
                boatTransitionTimer = 0.0f;
                boatTransitionStartBoat = boatPos;
                glm::vec3 toPlanet1 = glm::normalize(planets[1].center - planets[0].center);
                boatTransitionEnd = planets[1].center - toPlanet1 * planets[1].radius;
            }
        }

        // スターに触れたらゲームクリア（惑星2にいる時のみ）
        if (!gameClear) {
            float distToStar = glm::length(players[0].pos - starPos);
            if (distToStar < starTouchRadius) {
                gameClear = true;
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

        // 惑星描画（メッシュがあれば全メッシュ、なければ球）
        if (!planetMeshes.empty()) {
            for (size_t i = 0; i < planets.size(); i++) {
                glm::mat4 planetModel = glm::translate(glm::mat4(1.0f), planets[i].center) * glm::scale(glm::mat4(1.0f), glm::vec3(planets[i].radius));
                glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(planetModel));
                glUniform3f(locObjectColor, planets[i].color.x, planets[i].color.y, planets[i].color.z);
                for (const LoadedMesh& m : planetMeshes) {
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
            }
            glUniform1i(locUseTexture, 0);
        } else {
            glBindVertexArray(sphereVAO);
            glUniform1i(locUseTexture, 0);
            for (size_t i = 0; i < planets.size(); i++) {
                glm::mat4 planetModel = glm::translate(glm::mat4(1.0f), planets[i].center) * glm::scale(glm::mat4(1.0f), glm::vec3(planets[i].radius));
                glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(planetModel));
                glUniform3f(locObjectColor, planets[i].color.x, planets[i].color.y, planets[i].color.z);
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

        // 敵描画（enemy.obj を使い、惑星に立ててプレイヤー方向を向く）
        for (const EnemyState& e : enemies) {
            if (!e.alive) continue;
            glm::vec3 enemyUp = glm::normalize(e.pos - planets[e.planetIndex].center);
            glm::vec3 toPlayer = glm::normalize(players[0].pos - e.pos);
            float enemyFacingYaw = getYawFromDirection(enemyUp, toPlayer) + 3.14159265f;
            drawCharacter(e.pos, EnemyState::DrawScale, glm::vec3(0.0f, 1.0f, 0.0f), enemyUp, enemyFacingYaw, enemyMeshes);
        }

        // 鍵描画（敵全員撃破で出現、惑星に垂直・金色で表示）
        if (keyVisible) {
            glm::vec3 keyUp = glm::normalize(keyPos - planets[0].center);
            drawCharacter(keyPos, keyScale, keyColor, keyUp, 0.0f, keyMeshes, &keyColor);
        }

        // ボート描画（鍵取得で出現。惑星0または移動中のみ表示、到着後は非表示）
        if (keyObtained && (players[0].planetIndex == 0 || boatTransitionActive)) {
            int boatPlanet = players[0].planetIndex;
            glm::vec3 boatUp = glm::normalize(boatPos - planets[boatPlanet].center);
            drawCharacter(boatPos, boatScale, glm::vec3(0.4f, 0.25f, 0.1f), boatUp, 0.0f, boatMeshes);
        }

        // スター描画（惑星2に配置、ゲームクリア前のみ表示）
        if (!gameClear) {
            glm::vec3 starUp = glm::normalize(starPos - planets[1].center);
            glm::vec3 starColor(1.0f, 0.9f, 0.2f);
            drawCharacter(starPos, starScale, starColor, starUp, 0.0f, starMeshes);
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
        players[0].isDamagePrev = players[0].isDamaged;

        // バッファーを入れ替える
        glfwSwapBuffers(window);
        // std::cout << currentPlanetIndex << std::endl;
    }
		
	// ゲーム終了処理
    if (sdlController) {
        SDL_GameControllerClose(sdlController);
    }
    SDL_Quit();

    // 再生中の曲を止める（オーディオは開いたままだから他の曲を流せる）
    Mix_HaltMusic();
    // 曲データを解放する
    if (bgm) Mix_FreeMusic(bgm);
    // オーディオを閉じる（曲を流せなくなる）
    Mix_CloseAudio();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}