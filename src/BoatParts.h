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

    void SetCurrentPlanet(Planet* currentPlanet) { mCurrentPlanet = currentPlanet; }
    void SetMeshes(std::vector<struct LoadedMesh>* meshes) { mMeshes = meshes; }
    void SetIsActive(bool isActive) { mIsActive = isActive; }
    void SetModelPath(std::string modelPath) { mModelPath = modelPath; }

    class CollectableComponent* GetCollectableComponent() const { return mCollectableComponent; }
    std::vector<struct LoadedMesh>* GetMeshes() const { return mMeshes; }
    bool GetIsActive() const { return mIsActive; }
    std::string GetModelPath() const { return mModelPath; }

private:

private:
    class Planet* mCurrentPlanet;

    bool mIsActive;

    std::string mModelPath;

    std::vector<struct LoadedMesh>* mMeshes;
    class CollectableComponent* mCollectableComponent;
};