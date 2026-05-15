#include "Crystal.h"
#include "Game.h"
#include "component/DestructibleComponent.h"

Crystal::Crystal(Game* game)
    : Actor(game)
{
    AddDestructibleComponent();
}

void Crystal::AddDestructibleComponent() {
    std::unique_ptr<DestructibleComponent> destructibleComponent = std::make_unique<DestructibleComponent>(this, 100);
    mDestructibleComponent = destructibleComponent.get();
    AddComponent(std::move(destructibleComponent));
}

void Crystal::UpdateActor(float deltaTime)
{
    bool isDestroyed = mDestructibleComponent->GetIsDestroyed();
    if (isDestroyed && mIsActive)
        OnDestroyed();
}

void Crystal::OnDestroyed() {
    mIsActive = false;
    mGame->GetAudioSystem()->PlaySE("destroySE");
}