#include "Planet.h"
#include "Stage.h"
#include "Game.h"
#include "Player.h"
#include "BoatParts.h"

BoatParts::BoatParts(Game* game)
    : Actor(game)
    , mPos({32.0f, 8.0f, 0.0f})
    , mIsActive(true)
{
    std::unique_ptr<CollectableComponent> collectableComponent = std::make_unique<CollectableComponent>(this, 100);
    mCollectableComponent = collectableComponent.get();
    AddComponent(std::move(collectableComponent));
}

void BoatParts::UpdateActor(float deltaTime)
{
    if (mCollectableComponent->GetIsObtained() && mIsActive) {
        mIsActive = false;
        GetGame()->GetAudioSystem()->PlaySE("pickUpSE");
    }
    if (mCurrentPlanet->GetPlanetType() == Planet::PlanetType::Normal) {
        mUpVec = {0.0f, 1.0f, 0.0f};
    } else {
        mUpVec = glm::normalize(mPos - mCurrentPlanet->GetCenter());
    }
}