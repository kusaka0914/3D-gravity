#ifndef KEY_H
#define KEY_H

#include "Actor.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class Key : public Actor {
public:
    Key(class Game* game);
    void UpdateActor(float deltaTime) override;

    void SetIsActive(bool isActive) { mIsActive = isActive; }
    void SetIsObtained(bool isObtained) { mIsObtained = isObtained; }
    void SetCurrentPlanet(int currentPlanet) { mCurrentPlanet = currentPlanet; }
    void SetMeshes(const std::vector<struct LoadedMesh> meshes) { mMeshes = meshes; }

    glm::vec3 GetPos() const { return mPos; }
    bool GetIsActive() const { return mIsActive; }
    bool GetIsObtained() const { return mIsObtained; }
    int GetCurrentPlanet() const { return mCurrentPlanet; }
    const std::vector<struct LoadedMesh>& GetMeshes() const { return mMeshes; }

private:
    glm::vec3 mPos;
    bool mIsActive;
    bool mIsObtained;
    int mCurrentPlanet;

    std::vector<struct LoadedMesh> mMeshes;
};
#endif
