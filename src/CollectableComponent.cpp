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

void CollectableComponent::Update(float deltaTime)
{
    std::vector<Player*> players = GetOwner()->GetGame()->GetPlayers();
    if (!mIsObtained) {
        // プレイヤーが触れたら取得する
        for (auto player : players) {
            BoatParts* boatParts = dynamic_cast<BoatParts*>(GetOwner());

            float distTo;
            if (boatParts)
                distTo = glm::length(player->GetPos() - boatParts->GetPos()); 

            Key* key = dynamic_cast<Key*>(GetOwner());
            if (key)
                distTo = glm::length(player->GetPos() - key->GetPos()); 

            Star* star = dynamic_cast<Star*>(GetOwner());
            if (star)
                distTo = glm::length(player->GetPos() - star->GetPos()); 

            const float pickupRadius = 0.8f;
            float attackMotionTimer = player->GetAttackMotionTimer();
            if (distTo < pickupRadius && attackMotionTimer < 0.0f)
            {
                mIsObtained = true;
            }
        }
    }
}