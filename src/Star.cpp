#include "Star.h"
#include "Planet.h"
#include "Stage.h"
#include "Game.h"
#include "Player.h"
#include "Boat.h"
#include "GameProgressState.h"

Star::Star(Game* game)
    : Actor(game)
    , mIsActive(false)
    , mClearTimer(-1.0f)
{
    std::unique_ptr<CollectableComponent> collectableComponent = std::make_unique<CollectableComponent>(this, 100);
    mCollectableComponent = collectableComponent.get();
    AddComponent(std::move(collectableComponent));
}

void Star::UpdateActor(float deltaTime)
{
    if (mCollectableComponent->GetIsObtained() && mIsActive) {
        mIsActive = false;
        GetGame()->GetGameProgressState()->SetSceneState("StageClear");
        Mix_HaltMusic();
        GetGame()->GetAudioSystem()->PlaySE("clearSE");
        mClearTimer = 12.0f;
    }
    if (mClearTimer >= 0.0f) {
        mClearTimer -= deltaTime;
        if (mClearTimer < 0.0f) {
            GetGame()->SetFadeInTimer(1.0f);
            GetGame()->GetGameProgressState()->SetNextSceneState("Playing");
        }
    }
}