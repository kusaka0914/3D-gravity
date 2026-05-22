#include "CharacterActor.h"
#include "Game.h"
#include "system/PhysicsSystem.h"
#include <iostream>

CharacterActor::CharacterActor(Game* game)
    :Actor(game)
    , mOnGround(false)
    , mIsJudgeLanding(true)
    , mVelocity(0.0f)
{
    
}

void CharacterActor::UpdateActor(float deltaTime) {
    if (mIsJudgeLanding)
        JudgeLanding();
}

void CharacterActor::JudgeLanding() {
    constexpr float bodyOffset = 0.3f;

    const glm::vec3 frontOffset =  mFacingForwardVec * bodyOffset;
    const glm::vec3 backOffset  = -mFacingForwardVec * bodyOffset;

    if (mOnGround) {
        // 体中央でのレイキャスト
        if (TryLandByRay(glm::vec3(0.0f), glm::vec3(0.0f)))
            return;

        // 体後ろ側でのレイキャスト
        if (TryLandByRay(backOffset, frontOffset))
            return;

        // 体前側でのレイキャスト
        if (TryLandByRay(frontOffset, backOffset))
            return;
    } else {
        // 体前側でのレイキャスト
        if (TryLandByRay(frontOffset, backOffset))
            return;

        // 体中央でのレイキャスト
        if (TryLandByRay(glm::vec3(0.0f), glm::vec3(0.0f)))
            return;
    }
}

bool CharacterActor::TryLandByRay(const glm::vec3& rayOffset, const glm::vec3& hitPosCorrection) {
    const RayInfo rayInfo = CreateRayInfo(rayOffset);

    const btDiscreteDynamicsWorld* bulletWorld = mGame->GetPhysicsSystem()->GetBulletWorld();

    if (!bulletWorld)
        return false;

    btCollisionWorld::ClosestRayResultCallback rayCallback = rayInfo.rayCallback;

    bulletWorld->rayTest(rayInfo.rayFrom, rayInfo.rayTo, rayCallback);

    if (!rayCallback.hasHit())
        return false;

    const btVector3 hitPt = rayCallback.m_hitPointWorld;
    glm::vec3 hitPos(hitPt.x(), hitPt.y(), hitPt.z());

    OnLanded(hitPos + hitPosCorrection);
    return true;
}

CharacterActor::RayInfo CharacterActor::CreateRayInfo(const glm::vec3& rayOffset) const
{
    const glm::vec3 rayCenter = mPos + rayOffset;

    constexpr float margin = 0.1f;
    const glm::vec3 rayFromPos = rayCenter + mUpVec * margin;
    const glm::vec3 rayToPos   = rayCenter - mUpVec * margin;

    const btVector3 rayFrom(rayFromPos.x, rayFromPos.y, rayFromPos.z);
    const btVector3 rayTo(rayToPos.x, rayToPos.y, rayToPos.z);

    const btCollisionWorld::ClosestRayResultCallback rayCallback(rayFrom, rayTo);

    return { rayFrom, rayTo, rayCallback };
}

void CharacterActor::OnLanded(const glm::vec3& hitPos) {
    mPos = hitPos;
    mOnGround = true;
    mVelocity = glm::vec3(0.0f);
}

void CharacterActor::NotOnLanded() {
    mOnGround = false;
}

void CharacterActor::ApplyGravity(float deltaTime) {
    constexpr float gravity = 9.8f;
    mVelocity -= mUpVec * gravity * deltaTime;
    mPos += mVelocity * deltaTime;
}