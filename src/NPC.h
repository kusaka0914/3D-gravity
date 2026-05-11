#pragma once

#include "Mesh.h"
#include "Planet.h"
#include "CharacterActor.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

class NPC : public CharacterActor {
public:
    NPC(class Game* game);
    ~NPC();

    void ProcessActor() override;
    void UpdateActor(float deltaTime) override;
    
    glm::mat4 getNPCView(float cameraDistance, bool isFixed = false);

    void SetIsActive(bool isActive) { mIsActive = isActive; }
    void SetCurrentPlanetNum(int currentPlanetNum) { mCurrentPlanetNum = currentPlanetNum; }
    void SetFacingYaw(float facingYaw) { mFacingYaw = facingYaw; }
    void SetRadius(float radius) { mRadius = radius; }
    void SetName(std::string name) { mName = name; }
    void SetTalkableComponent(class TalkableComponent* talkableComponent) { mTalkableComponent = talkableComponent; }

    bool GetIsActive() const { return mIsActive; }
    const glm::vec3& GetForwardVec() const { return mForwardVec; }
    const glm::vec3& GetLeftVec() const { return mLeftVec; }
    const glm::vec3& GetFacingForwardVec() const { return mFacingForwardVec; }
    const glm::vec3& GetFacingLeftVec() const { return mFacingLeftVec; }
    int GetCurrentPlanetNum() const { return mCurrentPlanetNum; }
    float GetFacingYaw() const { return mFacingYaw; }
    float GetRadius() const { return mRadius; }
    std::string GetName() const { return mName; }

    class TalkableComponent* GetTalkableComponent() const { return mTalkableComponent; }

private:
    void UpdateWorldVec();

private:
    bool mIsActive;

    int mCurrentPlanetNum;

    float mFacingYaw;
    float mRadius;

    glm::vec3 mForwardVec;
    glm::vec3 mLeftVec;
    glm::vec3 mFacingForwardVec;
    glm::vec3 mFacingLeftVec;

    std::string mName;

    class TalkableComponent* mTalkableComponent;
};