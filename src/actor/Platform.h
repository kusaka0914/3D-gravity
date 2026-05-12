#pragma once

#include "actor/Actor.h"

class Platform : public Actor {
public:
    Platform(class Game* game);
    void UpdateActor(float deltaTime) override;

    void SetRadius(const float radius) { mRadius = radius; }

    float GetRadius() const { return mRadius; }

private:
    float mRadius;
};