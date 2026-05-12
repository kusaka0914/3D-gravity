#include "DestructibleComponent.h"
#include "Game.h"
#include "actor/Player.h"
#include "actor/Crystal.h"

DestructibleComponent::DestructibleComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mIsDestroyed(false)
    , mIsAttackedPrev(false)
    , mDestroyHp(0.0f)
{
}

void DestructibleComponent::Update(float deltaTime) {
    if (!mIsDestroyed) 
        TryDestroyed();
}

void DestructibleComponent::TryDestroyed() {
    glm::vec3 ownerPos = mOwner->GetPos();
    Crystal* ownerCrystal = dynamic_cast<Crystal*>(mOwner);
    float ownerRadius = ownerCrystal->GetRadius();
    
    std::vector<Player*> players = mOwner->GetGame()->GetPlayers();
    for (auto player : players) {
        float attackMotionTimer = player->GetAttackMotionTimer();
        if (attackMotionTimer < 0.0f) {
            mIsAttackedPrev = false;
        }
        
        glm::vec3 playerPos = player->GetPos();
        float distTo = glm::length(playerPos - ownerPos); 
        
        float strongAttackTimer = player->GetStrongAttackTimer();
        float attack = player->GetAttack();

        if (distTo >= ownerRadius + 2.0f) return;

        if (strongAttackTimer >= 0.0f || (attackMotionTimer >= 0.0f && !mIsAttackedPrev))
            OnDamaged(attack);
        
        if (mDestroyHp <= 0.0f)
            OnDestroyed();
    }
}

void DestructibleComponent::OnDamaged(const float attack) {
    mDestroyHp -= attack;
    mIsAttackedPrev = true;
}

void DestructibleComponent::OnDestroyed() {
    mDestroyHp = 0.0f;
    mIsDestroyed = true;
}