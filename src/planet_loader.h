#ifndef PLANET_LOADER_H
#define PLANET_LOADER_H

#include "planet.h"
#include <vector>

bool loadPlanetsFromYaml(const char* path, std::vector<Planet>& outPlanets);

#endif
