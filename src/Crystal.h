#pragma once

#include "Actor.h"
#include "Mesh.h"
#include "DestructibleComponent.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class Crystal : public Actor {
public:
    Crystal(class Game* game);
    void UpdateActor(float deltaTime) override;

    void SetRadius(const float radius) { mRadius = radius; }

    void SetIsActive(bool isActive) { mIsActive = isActive; }

    DestructibleComponent* GetDestructibleComponent() const { return mDestructibleComponent; }
    float GetRadius() const { return mRadius; }

    bool GetIsActive() const { return mIsActive; }

private:
    bool mIsActive;

    float mRadius;

    DestructibleComponent* mDestructibleComponent;
};