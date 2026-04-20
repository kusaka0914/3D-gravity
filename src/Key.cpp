#include "Key.h"
#include "Planet.h"
#include "Stage.h"
#include "Game.h"
#include "Player.h"
#include "Boat.h"

Key::Key(Game* game)
    : Actor(game)
    , mPos({0.0f, 8.0f, 0.0f})
{
    std::unique_ptr<CollectableComponent> collectableComponent = std::make_unique<CollectableComponent>(this, 100, true);
    mCollectableComponent = collectableComponent.get();
    AddComponent(std::move(collectableComponent));
}

void Key::UpdateActor(float deltaTime)
{
    // mUpVec = glm::normalize(mPos - currentPlanet->GetCenter());
    if (mCollectableComponent->GetIsObtained()) {
        std::vector<Boat*> boats = mCurrentPlanet->GetBoats();
        for (auto boat : boats) {
            boat->SetIsActive(true);
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