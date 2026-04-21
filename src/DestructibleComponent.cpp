#include "Game.h"
#include "Player.h"
#include "DestructibleComponent.h"

DestructibleComponent::DestructibleComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mIsDestroyed(false)
{
}

void DestructibleComponent::Update(float deltaTime)
{
    std::vector<Player*> players = GetOwner()->GetGame()->GetPlayers();
    if (!mIsDestroyed) {
        // 攻撃中のプレイヤーが触れたら破壊
        for (auto player : players) {
            float distTo = glm::length(player->GetPos() - GetOwner()->GetPos()); 
            if (player->GetAttackMotionTimer() < 0.0f) {
                mIsAttackedPrev = false;
            }
            if (distTo < GetOwner()->GetRadius() && player->GetAttackMotionTimer() > 0.0f && !mIsAttackedPrev)
            {
                mDestroyCount--;
                if (mDestroyCount <= 0) {
                    mDestroyCount = 0;
                    mIsDestroyed = true;
                    GetOwner()->SetIsActive(false);
                    GetOwner()->GetGame()->GetAudioSystem()->PlaySE("destroySE");
                }
                mIsAttackedPrev = true;
            }
        }
    }
}