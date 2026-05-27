#include "Key.h"
#include "Game.h"
#include "actor/Boat.h"
#include "actor/Planet.h"
#include "component/CollectableComponent.h"
#include "component/FocusComponent.h"
#include "system/AudioSystem.h"

Key::Key(Game* game) : Actor(game), mIsActivePrev(false)
{
    mIsActive = false;

    AddCollectableComponent();
    AddFocusComponent();
}

void Key::AddCollectableComponent()
{
    std::unique_ptr<CollectableComponent> collectableComponent = std::make_unique<CollectableComponent>(this, 100);
    mCollectableComponent = collectableComponent.get();
    AddComponent(std::move(collectableComponent));
}

void Key::AddFocusComponent()
{
    std::unique_ptr<FocusComponent> focusComponent = std::make_unique<FocusComponent>(this, 100);
    mFocusComponent = focusComponent.get();
    AddComponent(std::move(focusComponent));
}

void Key::UpdateActor(float deltaTime)
{
    const bool isJustShown = !mIsActivePrev && mIsActive;
    if (isJustShown) {
        OnShown();
    }

    const bool shouldStartOnObtained = mCollectableComponent->GetIsObtained() && mIsActive;
    if (shouldStartOnObtained) {
        OnObtained();
    }

    mIsActivePrev = mIsActive;
}

void Key::OnShown() const
{
    mGame->GetAudioSystem()->PlaySE("showKeySE");
}

void Key::OnObtained()
{
    mIsActive = false;

    for (auto boat : mCurrentPlanet->GetBoats()) {
        boat->GetFocusComponent()->StartFocus();
    }
}