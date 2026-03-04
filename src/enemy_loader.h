#ifndef ENEMY_LOADER_H
#define ENEMY_LOADER_H

#include "enemy.h"
#include "planet.h"
#include <vector>

bool loadEnemiesFromYaml(const char* path, const std::vector<Planet>& planets,
    std::vector<EnemyPtr>& outEnemies);

#endif
