#pragma once

#include "actor/Planet.h"
#include "actor/Enemy.h"
#include <glm/glm.hpp>
#include <yaml-cpp/yaml.h>
#include <vector>
#include <unordered_map>

class Loader {
public:
    // struct PlayerInfo {
    //     int currentPlanetNum = 0;
    //     float hp = 0.0f;
    //     float attack = 0.0f;
    //     float cameraPitch = 0.0f;
    //     std::string modelPath = "";
    // };
    Loader(class Game* game);
    // Initialize();
    bool LoadDataFromYaml(bool isLoadPlayer);
    
    Game* GetGame() const { return mGame; }
private:
    bool LoadPlayersFromYaml(const char* path);
    bool LoadNPCsFromYaml(const char* path);
    bool LoadEnemiesFromYaml(const char* path);
    bool LoadPlanetsFromYaml(const char* path);
    bool LoadBoatsFromYaml(const char* path);
    bool LoadBoatPartsFromYaml(const char* path);
    bool LoadKeysFromYaml(const char* path);
    bool LoadCrystalsFromYaml(const char* path);
    bool LoadStarFromYaml(const char* path);
    bool LoadPlatformsFromYaml(const char* path);

    glm::vec3 CalculatePos(YAML::Node node, Planet* currentPlanet);
    Game* mGame;
    // std::unordered_map<std::string, >
};