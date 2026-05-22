#pragma once

#include <glm/glm.hpp>
#include <yaml-cpp/yaml.h>

class Game;
class Planet;

class ActorLoadSystem {
public:
    ActorLoadSystem(Game* game);

    void LoadData(bool isLoadPlayer);

private:
    void LoadPlayers(const char* path);
    void LoadNPCs(const char* path);
    void LoadEnemies(const char* path);
    void LoadPlanets(const char* path);
    void LoadBoats(const char* path);
    void LoadBoatParts(const char* path);
    void LoadKeys(const char* path);
    void LoadCrystals(const char* path);
    void LoadStar(const char* path);
    void LoadPlatforms(const char* path);

    glm::vec3 CalculatePos(YAML::Node node, Planet* currentPlanet);

private:
    Game* mGame;
};