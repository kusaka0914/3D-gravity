#include "Game.h"
#include "Player.h"
#include "NPC.h"
#include "TalkableComponent.h"
#include <glm/glm.hpp>

TalkableComponent::TalkableComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mIsTalkable(false)
{
}

void TalkableComponent::Update(float deltaTime)
{
    std::vector<Player*> players = GetOwner()->GetGame()->GetPlayers();
    for (auto player : players) {
        NPC* npc = dynamic_cast<NPC*>(GetOwner());
        if (npc == nullptr) return;

        glm::vec3 playerPos = player->GetPos();
        glm::vec3 toNPC = glm::normalize(npc->GetPos() - playerPos);
        float npcYaw = player->getYawFromDirection(npc->GetUpVec(), toNPC);
        npc->SetFacingYaw(npcYaw);
        float talkableDist = 1.0f;
        float toPlayerDist = glm::length(playerPos - npc->GetPos());
        
        if (toPlayerDist <= talkableDist) {
            mIsTalkable = true;
            player->SetTalkingNPC(npc);
        } else {
            mIsTalkable = false;
            player->SetTalkingNPC(nullptr);
        }
    }
}