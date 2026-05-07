#include "Boat.h"
#include "Planet.h"
#include "Stage.h"
#include "Game.h"
#include "Player.h"
#include "Loader.h"
#include "UIState.h"
#include "FocusComponent.h"
#include "GameProgressState.h"

Boat::Boat(Game* game)
    : Actor(game)
    , mDestStage(0)
    , mIsMoving(false)
    , mIsActivePrev(false)
    , mIsActive(false)
    , mTransitionTimer(0.0f)
    , mProgress(0.0f)
    , mStartPos(mPos)
    , mDestPos(0.0f)
{
    std::unique_ptr<FocusComponent> focusComponent = std::make_unique<FocusComponent>(this, 100);
    mFocusComponent = focusComponent.get();
    AddComponent(std::move(focusComponent));
}

void Boat::Initialize() {
    glm::vec3 destPlanetCenter = mDestPlanet->GetPos();
    glm::vec3 boatPos = mPos;
    glm::vec3 toDestPlanet = glm::normalize(destPlanetCenter - boatPos);
    float destPlanetRadius = mDestPlanet->GetRadius();
    mDestPos = destPlanetCenter - toDestPlanet * (destPlanetRadius + 3.0f);
}

void Boat::UpdateActor(float deltaTime) {
    UpdateUpVec();

    if (!mIsActivePrev && mIsActive)
        OnShown();

    if (mIsMoving)
        UpdateMoving(deltaTime);

    mIsActivePrev = mIsActive;
}

void Boat::OnShown() {
    int currentStageNum = GetGame()->GetCurrentStageNum();
    if (currentStageNum != 0) {
        GetGame()->GetAudioSystem()->PlaySE("showBoatSE");
    }
}

void Boat::UpdateMoving(float deltaTime) {
    int currentStageNum = GetGame()->GetCurrentStageNum();
    if (currentStageNum == 0) {
        GetGame()->ChangeStage(mDestStage);
        GetGame()->SetFadeInTimer(1.0f);
        mIsMoving = false;
        return;
    }

    if (mProgress < 1.0f) {
        HandleMoving(deltaTime);
        return;
    }
    HandleArrived();
}

void Boat::HandleMoving(float deltaTime) {
    mTransitionTimer += deltaTime;

    float transitionDuration = 3.0f;
    mProgress = glm::min(1.0f, mTransitionTimer / transitionDuration);
    mProgress = mProgress * mProgress * (3.0f - 2.0f * mProgress);

    mPos = mStartPos + (mDestPos - mStartPos) * mProgress;
}

void Boat::HandleArrived() {

    mPos = mDestPos;
    mIsMoving = false;
    if (!GetGame()->GetUIState()->GetIsBattleTutorialShown()) {
        GetGame()->GetUIState()->SetCurrentTutorialKind("Battle");
        GetGame()->GetGameProgressState()->SetSceneState("Talking");
        GetGame()->GetUIState()->SetIsBattleTutorialShown(true);
    }
}