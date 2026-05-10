#pragma once
#include "Actor.h"

class CharacterActor : public Actor {
public:
    CharacterActor(class Game* game);
    void OnLanded(const glm::vec3& hitPos);
    void NotOnLanded();

    bool GetOnGround() const { return mOnGround; }
    bool GetIsJudgeLanding() const { return mIsJudgeLanding; }

protected:
    bool mOnGround;
    bool mIsJudgeLanding;

    glm::vec3 mVelocity;

    class JudgeLandingComponent* mJudgeLandingComponent;
};