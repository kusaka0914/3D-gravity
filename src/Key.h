#ifndef KEY_H
#define KEY_H

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

    void SetCurrentPlanet(class Planet* currentPlanet) { mCurrentPlanet = currentPlanet; }
    void SetPos(glm::vec3& pos) { mPos = pos; }
    void SetMeshes(const std::vector<struct LoadedMesh> meshes) { mMeshes = meshes; }

    const glm::vec3& GetPos() const override { return mPos; }
    class CollectableComponent* GetCollectableComponent() const { return mCollectableComponent; }
    const std::vector<struct LoadedMesh>& GetMeshes() const { return mMeshes; }

private:
    Planet* mCurrentPlanet;
    CollectableComponent* mCollectableComponent;
    glm::vec3 mPos;
    glm::vec3 mUpVec;

    std::vector<struct LoadedMesh> mMeshes;
};
#endif
