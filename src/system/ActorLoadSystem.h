#pragma once

#include "actor/Planet.h"
#include "actor/Enemy.h"
#include <glm/glm.hpp>
#include <yaml-cpp/yaml.h>
#include <vector>
#include <unordered_map>

class ActorLoadSystem {
public:
    ActorLoadSystem(class Game* game);

    void LoadData(bool isLoadPlayer);
    
    Game* GetGame() const { return mGame; }
private:
    bool LoadPlayers(const char* path);
    bool LoadNPCs(const char* path);
    bool LoadEnemies(const char* path);
    bool LoadPlanets(const char* path);
    bool LoadBoats(const char* path);
    bool LoadBoatParts(const char* path);
    bool LoadKeys(const char* path);
    bool LoadCrystals(const char* path);
    bool LoadStar(const char* path);
    bool LoadPlatforms(const char* path);

    glm::vec3 CalculatePos(YAML::Node node, Planet* currentPlanet);

private:
    Game* mGame;
};