#include "Game.h"
#include "Player.h"
#include "DestructibleComponent.h"

DestructibleComponent::DestructibleComponent(Actor* owner, int updateOrder, bool isActive)
    : Component(owner, updateOrder)
    , mIsActive(isActive)
    , mIsDestroyed(false)
{
}

void DestructibleComponent::Update(float deltaTime)
{
    std::vector<Player*> players = GetOwner()->GetGame()->GetPlayers();
    if (mIsActive) {
        // 攻撃中のプレイヤーが触れたら破壊
        for (auto player : players) {
            float distTo = glm::length(player->GetPos() - GetOwner()->GetPos()); 
            const float destroyRadius = 2.0f;
            if (distTo < destroyRadius && player->GetAttackMotionTimer() > 0.0f)
            {
                mIsActive = false;
                mIsDestroyed = true;
                GetOwner()->GetGame()->GetAudioSystem()->PlaySE("destroySE");
            }
        }
    }
}