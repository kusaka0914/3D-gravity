#include "Planet.h"
#include "Enemy.h"
#include <glm/glm.hpp>
#include <yaml-cpp/yaml.h>
#include <vector>

class Loader {
public:
    Loader(class Game* game);
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

    glm::vec3 CalculatePos(YAML::Node node, Planet* currentPlanet);
    Game* mGame;
};