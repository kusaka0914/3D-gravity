#include "Planet.h"
#include "Enemy.h"
#include <glm/glm.hpp>
#include <vector>

class Loader {
public:
    Loader(class Game* game);
    bool loadPlayersFromYaml(const char* path);
    bool loadEnemiesFromYaml(const char* path);
    bool loadPlanetsFromYaml(const char* path);
    bool loadBoatsFromYaml(const char* path);
    
    Game* GetGame() const { return mGame; }
private:
    Game* mGame;
};