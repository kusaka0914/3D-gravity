#pragma once

#include "actor/Actor.h"

class Game;
class DestructibleComponent;

class Crystal : public Actor {
public:
    Crystal(Game* game);
    void UpdateActor(float deltaTime) override;

    void SetRadius(const float radius) { mRadius = radius; }

    DestructibleComponent* GetDestructibleComponent() const { return mDestructibleComponent; }
    float GetRadius() const { return mRadius; }

private:
    void AddDestructibleComponent();
    void OnDestroyed();

private:
    float mRadius;

    DestructibleComponent* mDestructibleComponent;
};