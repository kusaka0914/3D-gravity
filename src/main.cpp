// 学習用にコメントをつけています。
#include "Game.h"
#include "shader.h"
#include "model_loader.h"
#include "planet.h"
#include "player.h"
#include "enemy.h"
#include "enemy_loader.h"
#include "planet_loader.h"
#include "player_loader.h"
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
    Game game;
    bool success = game.Initialize();
    if(success) {
        game.RunLoop();
        game.Shutdown();
    }
    return 0;
}