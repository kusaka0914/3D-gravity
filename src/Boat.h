#pragma once

#include "Actor.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class Boat : public Actor {
public:
    Boat(class Game* game);
    void Initialize() override;
    void UpdateActor(float deltaTime) override;

    void SetDestPlanet(Planet* destPlanet) { mDestPlanet = destPlanet; }
    void SetDestStage(int destStage) { mDestStage = destStage; }
    void SetIsMoving(bool isMoving) { mIsMoving = isMoving; }
    void SetIsActive(bool isActive) { mIsActive = isActive; }
    void SetTransitionTimer(float transitionTimer) { mTransitionTimer = transitionTimer; }
    void SetProgress(float progress) { mProgress = progress; }
    void SetStartPos(const glm::vec3& startPos) { mStartPos = startPos; }

    bool GetIsMoving() const { return mIsMoving; }
    bool GetIsActive() const { return mIsActive; }
    float GetTransitionTimer() const { return mTransitionTimer; }
    float GetProgress() const { return mProgress; }
    const glm::vec3& GetStartPos() const { return mStartPos; }
    const glm::vec3& GetDestPos() const { return mDestPos; }
    class FocusComponent* GetFocusComponent() const { return mFocusComponent; }

private:
    void OnShown();
    void UpdateMoving(float deltaTime);
    void HandleMoving(float deltaTime);
    void HandleArrived();

private:
    Planet* mDestPlanet;

    bool mIsMoving;
    bool mIsActivePrev;
    bool mIsActive;

    int mDestStage;
    
    float mTransitionTimer;
    float mProgress;

    glm::vec3 mStartPos;
    glm::vec3 mDestPos;

    class FocusComponent* mFocusComponent;
};