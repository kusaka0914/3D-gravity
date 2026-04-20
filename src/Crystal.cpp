#include "Crystal.h"
#include "Planet.h"
#include "Stage.h"
#include "Game.h"
#include "Player.h"
#include "Boat.h"

Crystal::Crystal(Game* game)
    : Actor(game)
    , mPos({0.0f, 8.0f, 0.0f})
    , mRadius(1.0f)
{
    std::unique_ptr<DestructibleComponent> destructibleComponent = std::make_unique<DestructibleComponent>(this, 100, true);
    mDestructibleComponent = destructibleComponent.get();
    AddComponent(std::move(destructibleComponent));
}

void Crystal::UpdateActor(float deltaTime)
{
    if (mDestructibleComponent->GetIsDestroyed()) {
        
    }
}