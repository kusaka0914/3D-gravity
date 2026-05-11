#pragma once

#include "Actor.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class BoatParts : public Actor {
public:
    BoatParts(class Game* game);
    void UpdateActor(float deltaTime) override;

    void SetIsActive(bool isActive) { mIsActive = isActive; }

    class CollectableComponent* GetCollectableComponent() const { return mCollectableComponent; }
    bool GetIsActive() const { return mIsActive; }

private:

private:
    bool mIsActive;

    class CollectableComponent* mCollectableComponent;
};