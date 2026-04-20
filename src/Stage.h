#include "Actor.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>

class Stage : public Actor {
public:
    Stage(class Game* game);
    void Initialize() override;
    void AddPlanet(class Planet* planet) { mPlanets.emplace_back(planet); }
    void AddPlanetMesh(std::string modelPath, std::vector<LoadedMesh> meshes) { mPlanetMeshesByPath[modelPath] = meshes; }
    void RemoveAllPlanet() { for(int i = 0; i < mPlanets.size(); i++) mPlanets.pop_back(); }

    const std::vector<class Planet*>& GetPlanets() const { return mPlanets; }
    const std::unordered_map<std::string, std::vector<LoadedMesh>>& GetPlanetMeshesByPath() const { return mPlanetMeshesByPath; }
private:
    std::vector<class Planet*> mPlanets;
    std::unordered_map<std::string, std::vector<LoadedMesh>> mPlanetMeshesByPath;
};