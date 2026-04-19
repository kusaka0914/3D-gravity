#include "Key.h"
#include "Planet.h"
#include "Stage.h"
#include "Game.h"
#include "Player.h"
#include "Boat.h"

Key::Key(Game* game)
    : Actor(game)
    , mPos({0.0f, 8.0f, 0.0f})
{
    std::unique_ptr<CollectableComponent> collectableComponent = std::make_unique<CollectableComponent>(this, 100, false);
    mCollectableComponent = collectableComponent.get();
    AddComponent(std::move(collectableComponent));
}

void Key::UpdateActor(float deltaTime)
{
    if (mCollectableComponent->GetIsObtained()) {
        std::vector<Boat*> boats = mCurrentPlanet->GetBoats();
        for (auto boat : boats) {
            boat->SetIsActive(true);
        }
    }
}