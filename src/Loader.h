#include "Planet.h"
#include "Enemy.h"
#include <glm/glm.hpp>
#include <vector>

/** プレイヤーの初期設定（YAML から読み込む用） */
struct PlayerInitialConfig {
    glm::vec3 pos{0};
    int planetIndex = 0;
    float hp = 100.0f;
    float attack = 10.0f;
    float cameraPitch = 0.4f;
};

class Loader {
public:
    Loader(class Game* game);
    bool loadPlayerFromYaml(const char* path, const std::vector<Planet*>& planets,
        PlayerInitialConfig& outConfig);
    bool loadEnemiesFromYaml(const char* path, const std::vector<Planet*>& planets,
        std::vector<class Enemy*>& outEnemies);
    bool loadPlanetsFromYaml(const char* path, std::vector<class Planet*>& outPlanets);
    
    Game* GetGame() const { return mGame; }
private:
    Game* mGame;
};