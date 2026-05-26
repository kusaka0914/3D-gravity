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

    void StartTravel();

    void SetDestPlanet(Planet* destPlanet) { mDestPlanet = destPlanet; }
    void SetDestStage(int destStage) { mDestStage = destStage; }

    bool GetIsMoving() const { return mIsMoving; }

    float GetProgress() const { return mProgress; }
    const glm::vec3& GetDestPos() const { return mDestPos; }
    FocusComponent* GetFocusComponent() const { return mFocusComponent; }

private:
    void AddFocusComponent();

    void OnShown();

    void UpdateMoving(float deltaTime);
    void UpdateMovement(float deltaTime);
    
    void FinishMoving();

    glm::vec3 CalculateDestPos();

private:
    Planet* mDestPlanet;

    bool mIsMoving;
    bool mIsActivePrev;

    int mDestStage;
    
    float mTransitionTimer;
    float mProgress;

    glm::vec3 mStartPos;
    glm::vec3 mDestPos;

    FocusComponent* mFocusComponent;
};