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
            if (player->GetAttackMotionTimer() < 0.0f) {
                mIsAttackedPrev = false;
            }
            float distTo = glm::length(player->GetPos() - GetOwner()->GetPos()); 
            if (distTo < GetOwner()->GetRadius() + 2.0f && player->GetStrongAttackTimer() >= 0.0f)
            {
                mDestroyHp -= player->GetAttack() * 5;
                mIsAttackedPrev = true;
            }
            if (distTo < GetOwner()->GetRadius() && player->GetAttackMotionTimer() >= 0.0f && !mIsAttackedPrev)
            {
                mDestroyHp -= player->GetAttack();
                mIsAttackedPrev = true;
            }
            if (mDestroyHp <= 0) {
                mDestroyHp = 0;
                mIsDestroyed = true;
                GetOwner()->SetIsActive(false);
                GetOwner()->GetGame()->GetAudioSystem()->PlaySE("destroySE");
            }
        }
    }
}