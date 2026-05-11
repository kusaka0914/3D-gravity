#pragma once

#include "Actor.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>
#include "CollectableComponent.h"

class Star : public Actor {
public:
    Star(class Game* game);
    void UpdateActor(float deltaTime) override;

    void SetIsActive(bool isActive) { mIsActive = isActive; }

    bool GetIsActive() const { return mIsActive; }
    class CollectableComponent* GetCollectableComponent() const { return mCollectableComponent; }

private:
    bool mIsActive;
    float mClearTimer;
    CollectableComponent* mCollectableComponent;
};