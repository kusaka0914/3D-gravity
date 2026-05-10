#include "CharacterActor.h"
#include "JudgeLandingComponent.h"

CharacterActor::CharacterActor(class Game* game)
    :Actor(game)
    ,mOnGround(false)
    ,mIsJudgeLanding(true)
    ,mVelocity(0.0f)
{
    std::unique_ptr<JudgeLandingComponent> judgeLandingComponent = std::make_unique<JudgeLandingComponent>(this, 100);
    mJudgeLandingComponent = judgeLandingComponent.get();
    AddComponent(std::move(judgeLandingComponent));
}

void CharacterActor::OnLanded(const glm::vec3& hitPos) {
    SetPos(hitPos);
    mOnGround = true;
    mVelocity = glm::vec3(0.0f);
}

void CharacterActor::NotOnLanded() {
    mOnGround = false;
}