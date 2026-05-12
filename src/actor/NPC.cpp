#include "NPC.h"
#include "Game.h"
#include "component/TalkableComponent.h"

NPC::NPC(Game* game)
    : CharacterActor(game)
    , mCurrentPlanetNum(0)
    , mFacingYaw(0.0f)
{
    mIsJudgeLanding = true;
}

NPC::~NPC()
{
    
}

void NPC::UpdateActor(float deltaTime)
{
    ApplyGravity(deltaTime);
}

void NPC::ApplyGravity(float deltaTime) {
    float gravity = 9.8f;
    mVelocity -= mUpVec * gravity * deltaTime;
    mPos += mVelocity * deltaTime;
}