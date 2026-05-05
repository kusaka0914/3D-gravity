#pragma once

#include "Actor.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>
#include "CollectableComponent.h"

class Key : public Actor {
public:
    Key(class Game* game);
    void UpdateActor(float deltaTime) override;

    void SetMeshes(std::vector<struct LoadedMesh>* meshes) { mMeshes = meshes; }
    void SetIsActive(bool isActive) { mIsActive = isActive; }

    class CollectableComponent* GetCollectableComponent() const { return mCollectableComponent; }
    std::vector<struct LoadedMesh>* GetMeshes() const { return mMeshes; }
    class FocusComponent* GetFocusComponent() const { return mFocusComponent; }
    bool GetIsActive() const { return mIsActive; }

private:
    CollectableComponent* mCollectableComponent;
    bool mIsActivePrev;
    bool mIsActive;

    std::vector<struct LoadedMesh>* mMeshes;
    class FocusComponent* mFocusComponent;
};