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

    void SetCurrentPlanet(class Planet* currentPlanet) { mCurrentPlanet = currentPlanet; }
    void SetMeshes(std::vector<struct LoadedMesh>* meshes) { mMeshes = meshes; }
    void SetRadius(const float radius) { mRadius = radius; }
    void SetScale(const float scale) { mScale = scale; }
    void SetIsActive(bool isActive) { mIsActive = isActive; }

    DestructibleComponent* GetDestructibleComponent() const { return mDestructibleComponent; }
    std::vector<struct LoadedMesh>* GetMeshes() const { return mMeshes; }
    float GetRadius() const { return mRadius; }
    float GetScale() const { return mScale; }
    bool GetIsActive() const { return mIsActive; }

private:
    bool mIsActive;

    float mRadius;
    float mScale;

    std::vector<struct LoadedMesh>* mMeshes;

    Planet* mCurrentPlanet;
    DestructibleComponent* mDestructibleComponent;
};