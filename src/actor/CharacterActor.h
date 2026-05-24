#pragma once
#include "actor/Actor.h"
#include <btBulletDynamicsCommon.h>

class Game;

class CharacterActor : public Actor {
public:
    struct RayInfo {
        btVector3 rayFrom;
        btVector3 rayTo;
        btCollisionWorld::ClosestRayResultCallback rayCallback;
    };

    CharacterActor(Game* game);

    void UpdateActor(float deltaTime) override;

    void Land(const glm::vec3& hitPos);
    void NotLand();

    void SetFacingYaw(float facingYaw) { mFacingYaw = facingYaw; }

    bool GetOnGround() const { return mOnGround; }
    bool GetIsJudgeLanding() const { return mIsJudgeLanding; }

    float GetFacingYaw() const { return mFacingYaw; }

    const glm::vec3& GetFacingForwardVec() const { return mFacingForwardVec; }

protected:
    void ApplyGravity(float deltaTime);

private:
    void JudgeLanding();
    bool TryLandByRay(const glm::vec3& rayOffset, const glm::vec3& hitPosCorrection);
    RayInfo CreateRayInfo(const glm::vec3& rayOffset) const;
    virtual void OnLanded() {};

protected:
    bool mOnGround;
    bool mIsJudgeLanding;

    float mFacingYaw;

    glm::vec3 mVelocity;
    glm::vec3 mFacingForwardVec;
};