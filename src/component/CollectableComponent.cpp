#include "CollectableComponent.h"
#include "Game.h"
#include "actor/Player.h"

CollectableComponent::CollectableComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mIsObtained(false)
{
}

void CollectableComponent::Update(float deltaTime) {
    if (!mIsObtained && IsCollectablePlayerInPickUpRadius())
        mIsObtained = true;
}

bool CollectableComponent::IsCollectablePlayerInPickUpRadius() const {
    const std::vector<Player*>& players = mOwner->GetGame()->GetPlayers();
    if (players.empty()) return false;

    glm::vec3 ownerPos = mOwner->GetPos();
    constexpr float pickupRadius = 0.8f;

    for (auto player : players) {
        if (player->GetActionState() == Player::ActionState::Attacking) continue;

        const float distTo = glm::length(player->GetPos() - ownerPos);    
        if (distTo <= pickupRadius)
            return true;
    }
    return false;
}