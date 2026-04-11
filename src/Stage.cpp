#include "Stage.h"
#include "Planet.h"
#include "Game.h"
#include <cmath>

Stage::Stage(Game* game)
    :Actor(game)
{
    Initialize();
}

void Stage::Initialize()
{
    auto planetUnique = std::make_unique<Planet>(GetGame());
    Planet* planet = planetUnique.get();
    GetGame()->AddActor(std::move(planetUnique));
    mPlanets.emplace_back(planet);
}