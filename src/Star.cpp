#include "Star.h"
#include "Planet.h"
#include "Stage.h"
#include "Game.h"
#include "Player.h"
#include "Boat.h"
#include "GameProgressState.h"

Star::Star(Game* game)
    : Actor(game)
    , mPos({32.0f, 8.0f, 0.0f})
    , mIsActive(false)
    , mCurrentPlanet(0)
{
    std::unique_ptr<CollectableComponent> collectableComponent = std::make_unique<CollectableComponent>(this, 100);
    mCollectableComponent = collectableComponent.get();
    AddComponent(std::move(collectableComponent));
}

void Star::UpdateActor(float deltaTime)
{
    if (mCollectableComponent->GetIsObtained() && mIsActive) {
        mIsActive = false;
        GetGame()->GetGameProgressState()->SetIsStageClear(true);
        Mix_HaltMusic();
        GetGame()->GetAudioSystem()->PlaySE("clearSE");
    }
}