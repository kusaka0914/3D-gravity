#include "Star.h"
#include "Planet.h"
#include "Stage.h"
#include "Game.h"
#include "Player.h"
#include "Boat.h"

Star::Star(Game* game)
    : Actor(game)
    , mPos({32.0f, 8.0f, 0.0f})
    , mIsActive(false)
    , mIsObtained(false)
    , mCurrentPlanet(0)
{
}

void Star::UpdateActor(float deltaTime)
{
    std::vector<class Player*> players = GetGame()->GetPlayers();
    if (GetIsActive()) {
        for (auto player : players) {
            
        }
    }
}