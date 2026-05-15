#pragma once
#include "actor/Actor.h"

class CharacterActor : public Actor {
public:
    CharacterActor(class Game* game);
    void OnLanded(const glm::vec3& hitPos);
    void NotOnLanded();

    void SetFacingYaw(float facingYaw) { mFacingYaw = facingYaw; }

    bool GetOnGround() const { return mOnGround; }
    bool GetIsJudgeLanding() const { return mIsJudgeLanding; }

    float GetFacingYaw() const { return mFacingYaw; }

private:
    void AddJudgeLandingComponent();

protected:
    bool mOnGround;
    bool mIsJudgeLanding;

    float mFacingYaw;

    glm::vec3 mVelocity;

    class JudgeLandingComponent* mJudgeLandingComponent;
};