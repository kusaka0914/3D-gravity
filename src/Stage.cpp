#include "Stage.h"
#include "Planet.h"
#include "Game.h"
#include <cmath>

Stage::Stage(Game* game)
    :Actor(game)
{
    
}

void Stage::Initialize()
{
    auto planetUnique = std::make_unique<Planet>(GetGame());
    planetUnique->SetCurrentStage(this);
    planetUnique->Initialize();
    Planet* planet = planetUnique.get();
    GetGame()->AddActor(std::move(planetUnique));
    mPlanets.emplace_back(planet);
}