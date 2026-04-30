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

    void SetPos(glm::vec3& pos) { mPos = pos; }
    void SetIsActive(bool isActive) override { mIsActive = isActive; }
    void SetCurrentPlanet(class Planet* currentPlanet) { mCurrentPlanet = currentPlanet; }
    void SetMeshes(const std::vector<struct LoadedMesh> meshes) { mMeshes = meshes; }

    const glm::vec3& GetPos() const override{ return mPos; }
    const glm::vec3& GetUpVec() const { return mUpVec; }
    bool GetIsActive() const { return mIsActive; }
    class Planet* GetCurrentPlanet() const { return mCurrentPlanet; }
    class CollectableComponent* GetCollectableComponent() const { return mCollectableComponent; }
    const std::vector<struct LoadedMesh>& GetMeshes() const { return mMeshes; }

private:
    Planet* mCurrentPlanet;
    glm::vec3 mPos;
    glm::vec3 mUpVec;
    bool mIsActive;
    float mClearTimer;
    CollectableComponent* mCollectableComponent;

    std::vector<struct LoadedMesh> mMeshes;
};