#include "FocusComponent.h"
#include "Game.h"
#include "actor/Actor.h"

FocusComponent::FocusComponent(Actor* owner, int updateOrder) : Component(owner, updateOrder), mFocusTimer(-1.0f) {}

void FocusComponent::Update(float deltaTime)
{
    if (mFocusTimer > 0.0f) {
        UpdateFocusTimer(deltaTime);
    }
}

void FocusComponent::UpdateFocusTimer(float deltaTime)
{
    mFocusTimer -= deltaTime;

    TryShowOwner();
    TryFinishFocus();
}

void FocusComponent::TryShowOwner()
{
    if (mOwner->GetIsActive()) {
        return;
    }

    constexpr float ownerAppearTime = 2.0f;
    const bool shouldOwnerAppear = mFocusTimer <= ownerAppearTime;
    if (shouldOwnerAppear) {
        mOwner->SetIsActive(true);
    }
}

void FocusComponent::TryFinishFocus()
{
    const bool shouldFinishFocus = mFocusTimer <= 0.0f;
    if (shouldFinishFocus) {
        mOwner->GetGame()->StartPlayingScene();
    }
}

void FocusComponent::StartFocus()
{
    mFocusTimer = 3.0f;
    mOwner->GetGame()->StartFocusingScene();
}