#include "Game.h"
#include "Player.h"
#include "CollectableComponent.h"

CollectableComponent::CollectableComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mIsObtained(false)
{
}

void CollectableComponent::Update(float deltaTime)
{
    std::vector<Player*> players = GetOwner()->GetGame()->GetPlayers();
    if (!mIsObtained) {
        // プレイヤーが触れたら取得して消す
        for (auto player : players) {
            float distTo = glm::length(player->GetPos() - GetOwner()->GetPos()); 
            const float pickupRadius = 0.8f;
            if (distTo < pickupRadius && player->GetAttackMotionTimer() < 0.0f)
            {
                mIsObtained = true;
                GetOwner()->SetIsActive(false);
            }
        }
    }
}