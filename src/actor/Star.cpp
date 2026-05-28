#include "Star.h"
#include "Game.h"
#include "component/CollectableComponent.h"

Star::Star(Game* game) : Actor(game)
{
    mIsActive = false;
    AddCollectableComponent();
}

void Star::AddCollectableComponent()
{
    std::unique_ptr<CollectableComponent> collectableComponent = std::make_unique<CollectableComponent>(this, 100);
    mCollectableComponent = collectableComponent.get();
    AddComponent(std::move(collectableComponent));
}

void Star::UpdateActor(float deltaTime)
{
    const bool shouldStartOnObtained = mCollectableComponent->GetIsObtained() && mIsActive;
    if (shouldStartOnObtained) {
        OnObtained();
    }
}

void Star::OnObtained()
{
    mIsActive = false;
    mGame->OnStarObtained();
}