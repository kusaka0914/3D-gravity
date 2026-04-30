#pragma once

#include "Actor.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>
#include "CollectableComponent.h"

class BoatParts : public Actor {
public:
    BoatParts(class Game* game);
    void UpdateActor(float deltaTime) override;

    void SetCurrentPlanet(Planet* currentPlanet) { mCurrentPlanet = currentPlanet; }
    void SetPos(glm::vec3& pos) { mPos = pos; }
    void SetMeshes(const std::vector<struct LoadedMesh> meshes) { mMeshes = meshes; }
    void SetIsActive(bool isActive) override { mIsActive = isActive; }
    void SetModelPath(std::string modelPath) { mModelPath = modelPath; }

    const glm::vec3& GetPos() const override { return mPos; }
    const glm::vec3& GetUpVec() const { return mUpVec; }
    class CollectableComponent* GetCollectableComponent() const { return mCollectableComponent; }
    const std::vector<struct LoadedMesh>& GetMeshes() const { return mMeshes; }
    bool GetIsActive() const { return mIsActive; }
    std::string GetModelPath() const { return mModelPath; }

private:
    class Planet* mCurrentPlanet;
    CollectableComponent* mCollectableComponent;
    glm::vec3 mPos;
    glm::vec3 mUpVec;
    bool mIsActive;
    std::string mModelPath;

    std::vector<struct LoadedMesh> mMeshes;
};