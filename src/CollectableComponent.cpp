#include "Game.h"
#include "Player.h"
#include "BoatParts.h"
#include "Key.h"
#include "Star.h"
#include "CollectableComponent.h"

CollectableComponent::CollectableComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mIsObtained(false)
{
}

void CollectableComponent::Update(float deltaTime) {
    if (mIsObtained)
        return;

    HandleCollected();
}

void CollectableComponent::HandleCollected() {
    std::vector<Player*> players = GetOwner()->GetGame()->GetPlayers();
    for (auto player : players) {
        glm::vec3 ownerPos = GetOwner()->GetPos();
        float distTo = glm::length(player->GetPos() - ownerPos);

        const float pickupRadius = 0.8f;
        float attackMotionTimer = player->GetAttackMotionTimer();
        if (distTo < pickupRadius && attackMotionTimer < 0.0f) {
            mIsObtained = true;
            break;
        }
    }
}