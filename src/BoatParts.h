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

    void SetMeshes(std::vector<struct LoadedMesh>* meshes) { mMeshes = meshes; }
    void SetIsActive(bool isActive) { mIsActive = isActive; }

    class CollectableComponent* GetCollectableComponent() const { return mCollectableComponent; }
    std::vector<struct LoadedMesh>* GetMeshes() const { return mMeshes; }
    bool GetIsActive() const { return mIsActive; }

private:

private:
    bool mIsActive;

    std::vector<struct LoadedMesh>* mMeshes;
    class CollectableComponent* mCollectableComponent;
};