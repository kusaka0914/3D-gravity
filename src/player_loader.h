#ifndef PLAYER_LOADER_H
#define PLAYER_LOADER_H

#include "planet.h"
#include <glm/glm.hpp>

/** プレイヤーの初期設定（YAML から読み込む用） */
struct PlayerInitialConfig {
    glm::vec3 pos{0};
    int planetIndex = 0;
    float hp = 100.0f;
    float attack = 10.0f;
    float cameraPitch = 0.4f;
};

bool loadPlayerFromYaml(const char* path, const std::vector<Planet>& planets,
    PlayerInitialConfig& outConfig);

#endif
