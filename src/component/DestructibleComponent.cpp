#include "DestructibleComponent.h"
#include "Game.h"
#include "actor/Player.h"
#include "system/AudioSystem.h"

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
    const glm::vec3 toOwner = mOwner->GetPos() - player->GetPos();
    const float distTo = glm::length(toOwner); 

    constexpr float hitRangeMargin = 2.0f;
    const float hitRange = mOwner->GetRadius() + hitRangeMargin;

    const glm::vec3 toOwnerN = glm::normalize(toOwner);
    const float dot = glm::dot(player->GetFacingForwardVec(), toOwnerN);
    float threshold = std::cos(player->GetAttackAngle() * 0.5f);

    return distTo <= hitRange && dot >= threshold;
}

void DestructibleComponent::ApplyDamage(float attack) {
    mIsHitCurrentAttack = true;
    mDestroyHp -= attack;
    if (mDestroyHp > 0.0f) {
        mOwner->GetGame()->GetAudioSystem()->PlaySE("attackSE");
        return;
    }

    mIsDestroyed = true;
}