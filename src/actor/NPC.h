#pragma once

#include "actor/CharacterActor.h"
#include <glm/glm.hpp>

class Game;
class TalkableComponent;

class NPC : public CharacterActor {
public:
    NPC(Game* game);
    ~NPC();

    void UpdateActor(float deltaTime) override;

    void SetCurrentPlanetNum(int currentPlanetNum) { mCurrentPlanetNum = currentPlanetNum; }
    void SetFacingYaw(float facingYaw) { mFacingYaw = facingYaw; }
    void SetRadius(float radius) { mRadius = radius; }
    void SetName(std::string name) { mName = name; }
    void SetTalkableComponent(TalkableComponent* talkableComponent) { mTalkableComponent = talkableComponent; }

    int GetCurrentPlanetNum() const { return mCurrentPlanetNum; }
    float GetFacingYaw() const { return mFacingYaw; }
    float GetRadius() const { return mRadius; }
    std::string GetName() const { return mName; }

    TalkableComponent* GetTalkableComponent() const { return mTalkableComponent; }

private:
    void ApplyGravity(float deltaTime);

private:
    int mCurrentPlanetNum;

    float mFacingYaw;
    float mRadius;

    std::string mName;

    TalkableComponent* mTalkableComponent;
};