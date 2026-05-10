#include "Planet.h"
#include "Stage.h"
#include "Game.h"
#include "Player.h"
#include "BoatParts.h"
#include "CollectableComponent.h"

BoatParts::BoatParts(Game* game)
    : Actor(game)
    , mIsActive(true)
{
    std::unique_ptr<CollectableComponent> collectableComponent = std::make_unique<CollectableComponent>(this, 100);
    mCollectableComponent = collectableComponent.get();
    AddComponent(std::move(collectableComponent));
}

void BoatParts::UpdateActor(float deltaTime) {
    if (mCollectableComponent->GetIsObtained() && mIsActive) {
        mIsActive = false;
        GetGame()->GetAudioSystem()->PlaySE("pickUpSE");
    }
}