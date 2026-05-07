#pragma once

#include "Actor.h"
#include "Mesh.h"
#include "DestructibleComponent.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class Platform : public Actor {
public:
    Platform(class Game* game);
    void UpdateActor(float deltaTime) override;

    void SetMeshes(std::vector<struct LoadedMesh>* meshes) { mMeshes = meshes; }
    void SetRadius(const float radius) { mRadius = radius; }
    void SetIsActive(bool isActive) { mIsActive = isActive; }

    std::vector<struct LoadedMesh>* GetMeshes() const { return mMeshes; }
    float GetRadius() const { return mRadius; }
    bool GetIsActive() const { return mIsActive; }

private:
    bool mIsActive;

    float mRadius;

    std::vector<struct LoadedMesh>* mMeshes;
};