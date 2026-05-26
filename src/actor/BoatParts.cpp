#include "BoatParts.h"
#include "Game.h"
#include "system/AudioSystem.h"
#include "component/CollectableComponent.h"

BoatParts::BoatParts(Game* game)
    : Actor(game)
{
    AddCollectableComponent();
}

void BoatParts::AddCollectableComponent() {
    std::unique_ptr<CollectableComponent> collectableComponent = std::make_unique<CollectableComponent>(this, 100);
    mCollectableComponent = collectableComponent.get();
    AddComponent(std::move(collectableComponent));
}

void BoatParts::UpdateActor(float deltaTime) {
    bool isObtained = mCollectableComponent->GetIsObtained();
    if (isObtained && mIsActive)
        OnObtained();
}

void BoatParts::OnObtained() {
    mIsActive = false;
    mGame->GetAudioSystem()->PlaySE("pickUpSE");
}