#include "Star.h"
#include "Game.h"
#include "state/GameProgressState.h"
#include "component/CollectableComponent.h"

Star::Star(Game* game)
    : Actor(game)
{
    mIsActive = false;

    AddCollectableComponent();
}

void Star::AddCollectableComponent() {
    std::unique_ptr<CollectableComponent> collectableComponent = std::make_unique<CollectableComponent>(this, 100);
    mCollectableComponent = collectableComponent.get();
    AddComponent(std::move(collectableComponent));
}

void Star::UpdateActor(float deltaTime)
{
    bool isObtained = mCollectableComponent->GetIsObtained();
    if (isObtained && mIsActive)
        OnObtained();
}

void Star::OnObtained() {
    mIsActive = false;
}