#include "Platform.h"
#include "Planet.h"
#include "Stage.h"
#include "Game.h"
#include "Player.h"
#include "Boat.h"

Platform::Platform(Game* game)
    : Actor(game)
    , mRadius(1.0f)
    , mIsActive(true)
{

}

void Platform::UpdateActor(float deltaTime)
{
    UpdateUpVec();
}