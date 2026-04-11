#include "Actor.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Stage : public Actor {
public:
    Stage(class Game* game);
    void Initialize();
    void AddPlanet(class Planet* planet) { mPlanets.emplace_back(planet); }
    const std::vector<class Planet*>& GetPlanets() const { return mPlanets; }
private:
    std::vector<class Planet*> mPlanets;
};