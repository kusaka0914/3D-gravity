#include "Boat.h"
#include "actor/Planet.h"
#include "Game.h"
#include "system/AudioSystem.h"
#include "component/FocusComponent.h"

Boat::Boat(Game* game)
    : Actor(game)
    , mDestStage(0)
    , mIsMoving(false)
    , mIsActivePrev(false)
    , mTransitionTimer(0.0f)
    , mProgress(0.0f)
    , mDestPos(0.0f)
{
    const int currentStage = mGame->GetCurrentStageNum();
    if (currentStage == 0)
        mIsActive = true;
    else 
        mIsActive = false;
    
    AddFocusComponent();
}

void Boat::AddFocusComponent() {
    std::unique_ptr<FocusComponent> focusComponent = std::make_unique<FocusComponent>(this, 100);
    mFocusComponent = focusComponent.get();
    AddComponent(std::move(focusComponent));
}

void Boat::Initialize() {
    mStartPos = mPos;
    mDestPos = CalculateDestPos();
}

glm::vec3 Boat::CalculateDestPos() {
    glm::vec3 destPlanetCenter = mDestPlanet->GetPos();
    glm::vec3 toDestPlanet = glm::normalize(destPlanetCenter - mPos);
    const float destPlanetRadius = mDestPlanet->GetRadius();

    glm::vec3 destPos = destPlanetCenter - toDestPlanet * (destPlanetRadius + 4.0f);
    return destPos;
}

void Boat::UpdateActor(float deltaTime) {
    bool isAllBoatPartsCollected = mCurrentPlanet->GetIsAllBoatPartsCollected();
    bool isAllEnemiesDead = mCurrentPlanet->GetIsAllEnemiesDead();
    bool isFocused = mFocusComponent->GetIsFocused();
    if ((isAllBoatPartsCollected || isAllEnemiesDead) && !isFocused)
        mFocusComponent->StartFocus();

    const int currentStageNum = mGame->GetCurrentStageNum();
    if (currentStageNum != 0 && !mIsActivePrev && mIsActive)
        OnShown();

    if (mIsMoving)
        UpdateMoving(deltaTime);

    mIsActivePrev = mIsActive;
}

void Boat::OnShown() {
    mGame->GetAudioSystem()->PlaySE("showBoatSE");
}

void Boat::UpdateMoving(float deltaTime) {
    if (mProgress < 1.0f) {
        UpdateMovement(deltaTime);
        return;
    }
    FinishMoving();
}

void Boat::UpdateMovement(float deltaTime) {
    mTransitionTimer += deltaTime;

    constexpr float transitionDuration = 3.0f;
    const float t = glm::min(1.0f, mTransitionTimer / transitionDuration);
    mProgress = t * t * (3.0f - 2.0f * t);

    mPos = mStartPos + (mDestPos - mStartPos) * mProgress;
}

void Boat::FinishMoving() {
    mPos = mDestPos;
    mIsMoving = false;

    mGame->OnBoatArrived(this);
}

void Boat::StartTravel() {
    if (mGame->GetCurrentStageNum() == 0) {
        mGame->OnBoatStageChangeRequested(mDestStage);
        return;
    }

    mIsMoving = true;
}