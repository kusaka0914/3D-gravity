#pragma once

#include "actor/Actor.h"
#include <glm/glm.hpp>

class Game;
class FocusComponent;

class Boat : public Actor {
public:
    Boat(Game* game);
    void Initialize() override;
    void UpdateActor(float deltaTime) override;

    void OnFocused();

    void SetIsMoving(bool isMoving) { mIsMoving = isMoving; }
    void SetIsArrived(bool isArrived) { mIsArrived = isArrived; }
    void SetIsChangeStage(bool isChangeStage) { mIsChangeStage = isChangeStage; }

    void SetDestPlanet(Planet* destPlanet) { mDestPlanet = destPlanet; }
    void SetDestStage(int destStage) { mDestStage = destStage; }
    void SetTransitionTimer(float transitionTimer) { mTransitionTimer = transitionTimer; }
    void SetProgress(float progress) { mProgress = progress; }
    void SetStartPos(const glm::vec3& startPos) { mStartPos = startPos; }

    bool GetIsMoving() const { return mIsMoving; }
    bool GetIsArrived() const { return mIsArrived; }
    bool GetIsChangeStage() const { return mIsChangeStage; }

    int GetDestStage() const { return mDestStage; }

    float GetTransitionTimer() const { return mTransitionTimer; }
    float GetProgress() const { return mProgress; }
    const glm::vec3& GetStartPos() const { return mStartPos; }
    const glm::vec3& GetDestPos() const { return mDestPos; }
    FocusComponent* GetFocusComponent() const { return mFocusComponent; }

private:
    void AddFocusComponent();
    void InitDestPos();
    void OnShown();
    void UpdateMoving(float deltaTime);
    void HandleMoving(float deltaTime);
    void HandleArrived();

private:
    Planet* mDestPlanet;

    bool mIsMoving;
    bool mIsActivePrev;
    bool mIsArrived;
    bool mIsChangeStage;

    int mDestStage;
    
    float mTransitionTimer;
    float mProgress;

    glm::vec3 mStartPos;
    glm::vec3 mDestPos;

    FocusComponent* mFocusComponent;
};