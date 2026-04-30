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
        float talkableDist = 1.0f;
        glm::vec3 playerPos = player->GetPos();
        float toPlayerDist = glm::length(playerPos - GetOwner()->GetPos());
        
        if (toPlayerDist <= talkableDist) {
            mIsTalkable = true;
            NPC* npc = dynamic_cast<NPC*>(GetOwner());
            if (npc != nullptr) {
                player->SetTalkingNPC(npc);
            }
        } else {
            mIsTalkable = false;
            player->SetTalkingNPC(nullptr);
        }
    }
}