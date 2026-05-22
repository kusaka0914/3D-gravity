#include "FocusComponent.h"
#include "Game.h"
#include "actor/Actor.h"

FocusComponent::FocusComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mFocusTimer(-1.0f)
    , mIsFocused(false)
{
}

void FocusComponent::Update(float deltaTime)
{
    if (mFocusTimer <= 0.0f) return;

    mFocusTimer -= deltaTime;
    if (mFocusTimer <= 0.0f)
        mOwner->GetGame()->StartPlayingScene();
    
    constexpr float ownerAppearTime = 2.0f;
    if (mFocusTimer <= ownerAppearTime)
        mOwner->SetIsActive(true);
}

void FocusComponent::StartFocus() {
    mFocusTimer = 3.0f;
    mIsFocused = true;
    mOwner->GetGame()->StartFocusingScene();
}