#include "Key.h"
#include "Game.h"
#include "system/AudioSystem.h"
#include "actor/Boat.h"
#include "actor/Planet.h"
#include "component/FocusComponent.h"
#include "component/CollectableComponent.h"

Key::Key(Game* game)
    : Actor(game)
    , mIsActivePrev(false)
{
    mIsActive = false;

    AddCollectableComponent();
    AddFocusComponent();
}

void Key::AddCollectableComponent() {
    std::unique_ptr<CollectableComponent> collectableComponent = std::make_unique<CollectableComponent>(this, 100);
    mCollectableComponent = collectableComponent.get();
    AddComponent(std::move(collectableComponent));
}

void Key::AddFocusComponent() {
    std::unique_ptr<FocusComponent> focusComponent = std::make_unique<FocusComponent>(this, 100);
    mFocusComponent = focusComponent.get();
    AddComponent(std::move(focusComponent));
}

void Key::UpdateActor(float deltaTime)
{
    // bool isAllEnemiesDead = mCurrentPlanet->GetIsAllEnemiesDead();
    // bool isFocused = mFocusComponent->GetIsFocused();
    // if (isAllEnemiesDead && !isFocused)
    //     mFocusComponent->StartFocus();

    if (!mIsActivePrev && mIsActive) 
        OnShown();

    bool isObtained = mCollectableComponent->GetIsObtained();
    if (isObtained && mIsActive)
        OnObtained();
}

void Key::OnShown() {
    mGame->GetAudioSystem()->PlaySE("showKeySE");
    mIsActivePrev = true;
}

void Key::OnObtained() {
    mIsActive = false;

    std::vector<Boat*> boats = mCurrentPlanet->GetBoats();
    for (auto boat : boats) {
        boat->GetFocusComponent()->StartFocus();
    }
}