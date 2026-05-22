#include "DestructibleComponent.h"
#include "Game.h"
#include "actor/Player.h"

DestructibleComponent::DestructibleComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mIsDestroyed(false)
    , mIsHitCurrentAttack(false)
    , mDestroyHp(0.0f)
{

}

void DestructibleComponent::Update(float deltaTime) {
    if (!mIsDestroyed)
        TryApplyDamage();
}

void DestructibleComponent::TryApplyDamage() {
    const std::vector<Player*>& players = mOwner->GetGame()->GetPlayers();

    for (auto player : players) {
        const Player::ActionState playerActionState = player->GetActionState();
        const bool isPlayerAttacking = playerActionState == Player::ActionState::StrongAttacking || playerActionState == Player::ActionState::Attacking;

        if (!isPlayerAttacking) {
            mIsHitCurrentAttack = false;
            continue;
        }

        if (!IsPlayerInHitRange(player) || mIsHitCurrentAttack) continue;

        ApplyDamage(player->GetAttack());
    }
}

bool DestructibleComponent::IsPlayerInHitRange(Player* player) const {
    constexpr float hitRangeMargin = 2.0f;
    const float hitRange = mOwner->GetRadius() + hitRangeMargin;

    const float distTo = glm::length(player->GetPos() - mOwner->GetPos()); 
    return distTo <= hitRange;
}

void DestructibleComponent::ApplyDamage(float attack) {
    mIsHitCurrentAttack = true;
    mDestroyHp -= attack;

    if (mDestroyHp <= 0.0f)
        mIsDestroyed = true;
}