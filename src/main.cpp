#include "Game.h"
#include <iostream>

// const unsigned int SCR_WIDTH = 800;
// const unsigned int SCR_HEIGHT = 600;

// // #region agent log
// static const char* const DEBUG_LOG_PATH = "/Users/kusakatakumi/Desktop/desktop_latest/engine/.cursor/debug-a01f92.log";
// static void debug_log(const char* hypothesisId, const char* message, const std::string& dataJson) {
//     std::ofstream f(DEBUG_LOG_PATH, std::ios::app);
//     if (!f) return;
//     f << "{\"sessionId\":\"a01f92\",\"hypothesisId\":\"" << hypothesisId << "\",\"location\":\"Main.cpp\",\"message\":\"" << message << "\",\"data\":" << dataJson << ",\"timestamp\":" << (unsigned long long)(glfwGetTime() * 1000.0) << "}\n";
//     f.close();
// }
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