#pragma once

#include "actor/Actor.h"
#include "system/Mesh.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>

class Stage {
public:
    Stage();
    void Initialize();
    void AddPlanet(class Planet* planet) { mPlanets.emplace_back(planet); }
    void AddPlanetMesh(std::string modelPath, std::vector<struct LoadedMesh>* meshes) { mPlanetMeshesByPath[modelPath] = meshes; }
    void RemoveAllPlanet() { mPlanets.clear(); }

    const std::vector<class Planet*>& GetPlanets() const { return mPlanets; }
    const std::unordered_map<std::string, std::vector<struct LoadedMesh>*>& GetPlanetMeshesByPath() const { return mPlanetMeshesByPath; }
private:
    std::vector<class Planet*> mPlanets;
    std::unordered_map<std::string, std::vector<struct LoadedMesh>*> mPlanetMeshesByPath;
};