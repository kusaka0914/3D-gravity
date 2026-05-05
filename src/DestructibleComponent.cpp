#include "Game.h"
#include "Player.h"
#include "DestructibleComponent.h"
#include "Crystal.h"

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
            Crystal* ownerCrystal = dynamic_cast<Crystal*>(GetOwner());
            float distTo = glm::length(player->GetPos() - ownerCrystal->GetPos()); 
            if (distTo < ownerCrystal->GetRadius() + 2.0f && player->GetStrongAttackTimer() >= 0.0f)
            {
                mDestroyHp -= player->GetAttack() * 5;
                mIsAttackedPrev = true;
            }
            if (distTo < ownerCrystal->GetRadius() && player->GetAttackMotionTimer() >= 0.0f && !mIsAttackedPrev)
            {
                mDestroyHp -= player->GetAttack();
                mIsAttackedPrev = true;
            }
            if (mDestroyHp <= 0) {
                mDestroyHp = 0;
                mIsDestroyed = true;
                ownerCrystal->SetIsActive(false);
                GetOwner()->GetGame()->GetAudioSystem()->PlaySE("destroySE");
            }
        }
    }
}