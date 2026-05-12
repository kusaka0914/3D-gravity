#include "Boat.h"
#include "actor/Planet.h"
#include "Game.h"
#include "component/FocusComponent.h"

Boat::Boat(Game* game)
    : Actor(game)
    , mDestStage(0)
    , mIsMoving(false)
    , mIsArrived(false)
    , mIsChangeStage(false)
    , mIsActivePrev(false)
    , mTransitionTimer(0.0f)
    , mProgress(0.0f)
    , mDestPos(0.0f)
{
    int currentStage = mGame->GetCurrentStageNum();
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
    InitDestPos();
}

void Boat::InitDestPos() {
    glm::vec3 destPlanetCenter = mDestPlanet->GetPos();
    glm::vec3 toDestPlanet = glm::normalize(destPlanetCenter - mPos);
    float destPlanetRadius = mDestPlanet->GetRadius();
    mDestPos = destPlanetCenter - toDestPlanet * (destPlanetRadius + 3.0f);
}

void Boat::UpdateActor(float deltaTime) {
    bool isAllBoatPartsCollected = mCurrentPlanet->GetIsAllBoatPartsCollected();
    bool isFocused = mFocusComponent->GetIsFocused();
    if (isAllBoatPartsCollected && !isFocused)
        OnFocused();

    if (!mIsActivePrev && mIsActive)
        OnShown();

    if (mIsMoving)
        UpdateMoving(deltaTime);

    mIsActivePrev = mIsActive;
}

void Boat::OnFocused() {
    mFocusComponent->SetFocusTimer(3.0f);
    mFocusComponent->SetIsFocused(true);
}

void Boat::OnShown() {
    int currentStageNum = GetGame()->GetCurrentStageNum();
    if (currentStageNum == 0) return;

    GetGame()->GetAudioSystem()->PlaySE("showBoatSE");
}

void Boat::UpdateMoving(float deltaTime) {
    if (mProgress < 1.0f) {
        HandleMoving(deltaTime);
        return;
    }
    HandleArrived();
}

void Boat::HandleMoving(float deltaTime) {
    mTransitionTimer += deltaTime;

    float transitionDuration = 3.0f;
    float t = glm::min(1.0f, mTransitionTimer / transitionDuration);
    mProgress = t * t * (3.0f - 2.0f * t);

    mPos = mStartPos + (mDestPos - mStartPos) * mProgress;
}

void Boat::HandleArrived() {
    mPos = mDestPos;
    mIsMoving = false;
    mIsArrived = true;
}