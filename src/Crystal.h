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
    void SetPos(glm::vec3& pos) { mPos = pos; }
    void SetMeshes(const std::vector<struct LoadedMesh> meshes) { mMeshes = meshes; }
    void SetRadius(const float radius) { mRadius = radius; }
    void SetScale(const float scale) { mScale = scale; }
    void SetIsActive(bool isActive) { mIsActive = isActive; }

    const glm::vec3& GetPos() const override { return mPos; }
    const glm::vec3& GetUpVec() const { return mUpVec; }
    DestructibleComponent* GetDestructibleComponent() const { return mDestructibleComponent; }
    const std::vector<struct LoadedMesh>& GetMeshes() const { return mMeshes; }
    float GetRadius() const override { return mRadius; }
    float GetScale() const { return mScale; }
    bool GetIsActive() const { return mIsActive; }

private:
    Planet* mCurrentPlanet;
    DestructibleComponent* mDestructibleComponent;
    glm::vec3 mPos;
    glm::vec3 mUpVec;
    float mRadius;
    float mScale;
    bool mIsActive;

    std::vector<struct LoadedMesh> mMeshes;
};