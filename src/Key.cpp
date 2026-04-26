#include "Key.h"
#include "Planet.h"
#include "Stage.h"
#include "Game.h"
#include "Player.h"
#include "Boat.h"
#include "FocusComponent.h"

Key::Key(Game* game)
    : Actor(game)
    , mPos({0.0f, 8.0f, 0.0f})
    , mIsActivePrev(false)
    , mIsActive(false)
{
    std::unique_ptr<CollectableComponent> collectableComponent = std::make_unique<CollectableComponent>(this, 100);
    mCollectableComponent = collectableComponent.get();
    AddComponent(std::move(collectableComponent));
    std::unique_ptr<FocusComponent> focusComponent = std::make_unique<FocusComponent>(this, 100);
    mFocusComponent = focusComponent.get();
    AddComponent(std::move(focusComponent));
}

void Key::UpdateActor(float deltaTime)
{
    if (!mIsActivePrev && mIsActive) {
        GetGame()->GetAudioSystem()->PlaySE("showKeySE");
        mIsActivePrev = true;
    }
    if (mCollectableComponent->GetIsObtained() && mIsActive) {
        std::vector<Boat*> boats = mCurrentPlanet->GetBoats();
        for (auto boat : boats) {
            boat->GetFocusComponent()->SetFocusTimer(3.0f);
            mIsActive = false;
        }
    }
}

// glm::mat4 Key::getKeyView() {
//     const float cameraDistance = 12.0f;
//     glm::vec3 back = glm::normalize(-mForwardVec);
//     glm::vec3 cameraDir = glm::normalize(std::cos(mCameraPitch) * back + std::sin(mCameraPitch) * mUpVec);
//     glm::vec3 cameraPos = mPos - cameraDir * cameraDistance;
//     return glm::lookAt(cameraPos, mPos, mUpVec);
// }