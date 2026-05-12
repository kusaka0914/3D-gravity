#include "CollectableComponent.h"
#include "Game.h"
#include "actor/Player.h"

CollectableComponent::CollectableComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mIsObtained(false)
{
}

void CollectableComponent::Update(float deltaTime) {
    if (!mIsObtained)
        TryCollect();
}

void CollectableComponent::TryCollect() {
    glm::vec3 ownerPos = mOwner->GetPos();
    const float pickupRadius = 0.8f;

    std::vector<Player*> players = mOwner->GetGame()->GetPlayers();
    for (auto player : players) {
        glm::vec3 playerPos = player->GetPos();
        float distTo = glm::length(playerPos - ownerPos);
        
        float attackMotionTimer = player->GetAttackMotionTimer();
        if (distTo >= pickupRadius || attackMotionTimer >= 0.0f) continue;

        mIsObtained = true;
        break;   
    }
}