#include "TalkableComponent.h"
#include "Game.h"
#include "actor/Player.h"
#include "actor/NPC.h"
#include "utils/MathUtils.h"
#include <glm/glm.hpp>

TalkableComponent::TalkableComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mIsTalkable(false)
{
}

void TalkableComponent::Update(float deltaTime)
{
    std::vector<Player*> players = mOwner->GetGame()->GetPlayers();
    if (players.empty()) return;

    NPC* npc = dynamic_cast<NPC*>(mOwner);
    if (npc == nullptr) return;

    for (auto player : players) {
        if (IsPlayerInTalkableRange(player)) {
            mIsTalkable = true;
            player->SetTalkingNPC(npc);
        } else {
            mIsTalkable = false;
            player->SetTalkingNPC(nullptr);
        }
    }
}

bool TalkableComponent::IsPlayerInTalkableRange(Player* player) {
    NPC* npc = dynamic_cast<NPC*>(mOwner);
    if (npc == nullptr) return false;

    float toPlayerDist = glm::length(player->GetPos() - npc->GetPos());
    constexpr float talkableDist = 1.0f;
    
    if (toPlayerDist <= talkableDist)
        return true;
    
    return false;
}