#include "Planet.h"
#include "Enemy.h"
#include <glm/glm.hpp>
#include <yaml-cpp/yaml.h>
#include <vector>

class Loader {
public:
    Loader(class Game* game);
    bool loadPlayersFromYaml(const char* path);
    bool loadEnemiesFromYaml(const char* path);
    bool loadPlanetsFromYaml(const char* path);
    bool loadBoatsFromYaml(const char* path);
    bool loadBoatPartsFromYaml(const char* path);
    bool loadKeysFromYaml(const char* path);
    
    Game* GetGame() const { return mGame; }
private:
    glm::vec3 CalculatePos(YAML::Node node, Planet* currentPlanet);
    Game* mGame;
};