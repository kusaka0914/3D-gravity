#include "Game.h"
#include "Player.h"
#include "DestructibleComponent.h"
#include "Crystal.h"

DestructibleComponent::DestructibleComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mIsDestroyed(false)
{
}

void DestructibleComponent::Update(float deltaTime)　{
    if (!mIsDestroyed) {
        HandleDestroyed();
    }
}

void DestructibleComponent::HandleDestroyed() {
    std::vector<Player*> players = GetOwner()->GetGame()->GetPlayers();
    for (auto player : players) {
        float attackMotionTimer = player->GetAttackMotionTimer();
        if (attackMotionTimer < 0.0f) {
            mIsAttackedPrev = false;
        }
        
        glm::vec3 ownerPos = GetOwner()->GetPos();
        float distTo = glm::length(player->GetPos() - ownerPos); 
        Crystal* ownerCrystal = dynamic_cast<Crystal*>(GetOwner());
        float ownerRadius = ownerCrystal->GetRadius();
        float strongAttackTimer = player->GetStrongAttackTimer();

        float attack = player->GetAttack();
        if (distTo < ownerRadius + 2.0f && strongAttackTimer >= 0.0f) {
            mDestroyHp -= attack * 5;
            mIsAttackedPrev = true;
        }
        
        if (distTo < ownerRadius && attackMotionTimer >= 0.0f && !mIsAttackedPrev) {
            mDestroyHp -= attack;
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