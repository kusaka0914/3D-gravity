#include "NPC.h"
#include "Game.h"
#include "Player.h"
#include "utils/MathUtils.h"

NPC::NPC(Game* game)
    : CharacterActor(game)
    , mIsTalkable(false)
{
    mIsJudgeLanding = true;
}

NPC::~NPC()
{
    
}

void NPC::UpdateActor(float deltaTime)
{
    CharacterActor::UpdateActor(deltaTime);
    LookNearestPlayer();
    CheckTalkable();

    if (!mOnGround)
        ApplyGravity(deltaTime);
}

void NPC::LookNearestPlayer() {
    Player* nearestPlayer = FindNearestPlayer();

    glm::vec3 toNearestPlayer = glm::normalize(nearestPlayer->GetPos() - mPos);
    mFacingForwardVec = toNearestPlayer;
    mFacingYaw = mGame->GetMathUtils()->GetYawFromDirection(mUpVec, toNearestPlayer) + 3.14159265f;
}

Player* NPC::FindNearestPlayer() {
    const std::vector<Player*>& players = mGame->GetPlayers();

    Player* nearestPlayer;
    float nearestToPlayerDist = 1e9;

    for (auto player : players) {
        const float toPlayerDist = glm::length(player->GetPos() - mPos);
        if (toPlayerDist <= nearestToPlayerDist) {
            nearestToPlayerDist = toPlayerDist;
            nearestPlayer = player;
        }
    }

    return nearestPlayer;
}

void NPC::CheckTalkable() {
    const std::vector<Player*>& players = mGame->GetPlayers();
    if (players.empty()) return;

    for (auto player : players) {
        if (IsPlayerInTalkableRange(player)) {
            mIsTalkable = true;
            player->SetTalkableNPC(this);
        } else {
            mIsTalkable = false;
        }
    }
}

bool NPC::IsPlayerInTalkableRange(Player* player) const {
    const float toPlayerDist = glm::length(player->GetPos() - mPos);
    constexpr float talkableRangeMargin = 0.5f;
    const float talkableRange = mRadius + talkableRangeMargin;
        
    return toPlayerDist <= talkableRange;
}