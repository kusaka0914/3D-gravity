#include <glm/glm.hpp>
#include <string>
#include <vector>

class Stage : Actor {
public:
    Stage(class Game* game);
    void AddPlanet(std::unique_ptr<class Planet> planet) { mPlanets.push_back(planet); }
    std::vector<std::unique_ptr<clas  Planet>>& GetPlanets() { return mPlanets; }
private:
    std::vector<std::unique_ptr<class Planet>> mPlanets;
};