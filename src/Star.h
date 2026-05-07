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
    void SetMeshes(std::vector<struct LoadedMesh>* meshes) { mMeshes = meshes; }

    bool GetIsActive() const { return mIsActive; }
    class CollectableComponent* GetCollectableComponent() const { return mCollectableComponent; }
    std::vector<struct LoadedMesh>* GetMeshes() const { return mMeshes; }

private:
    bool mIsActive;
    float mClearTimer;
    CollectableComponent* mCollectableComponent;

    std::vector<struct LoadedMesh>* mMeshes;
};