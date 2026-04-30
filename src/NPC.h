#pragma once

#include "Mesh.h"
#include "Planet.h"
#include "Actor.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

class NPC : public Actor {
public:
    NPC(class Game* game);
    ~NPC();

    void ProcessActor() override;
    void UpdateActor(float deltaTime) override;
    
    float getYawFromDirection(const glm::vec3& up, const glm::vec3& dir);
    glm::mat4 getNPCView(float cameraDistance, bool isFixed = false);

    void SetIsActive(bool isActive) { mIsActive = isActive; }
    void SetCurrentPlanet(Planet* currentPlanet) { mCurrentPlanet = currentPlanet; }
    void SetPos(const glm::vec3& pos) { mPos = pos; }
    void SetUpVec(const glm::vec3& upVec) { mUpVec = upVec; }
    void SetCurrentPlanetNum(int currentPlanetNum) { mCurrentPlanetNum = currentPlanetNum; }
    void SetFacingYaw(float facingYaw) { mFacingYaw = facingYaw; }
    void SetRadius(float radius) { mRadius = radius; }
    void SetModelPath(std::string modelPath) { mModelPath = modelPath; }
    void SetName(std::string name) { mName = name; }
    void SetMeshes(const std::vector<struct LoadedMesh> meshes) { mMeshes = meshes; }

    bool GetIsActive() const { return mIsActive; }
    Planet* GetCurrentPlanet() const { return mCurrentPlanet; }
    const glm::vec3& GetPos() const override { return mPos; }
    const glm::vec3& GetUpVec() const { return mUpVec; }
    const glm::vec3& GetForwardVec() const { return mForwardVec; }
    const glm::vec3& GetLeftVec() const { return mLeftVec; }
    const glm::vec3& GetFacingForwardVec() const { return mFacingForwardVec; }
    const glm::vec3& GetFacingLeftVec() const { return mFacingLeftVec; }
    int GetCurrentPlanetNum() const { return mCurrentPlanetNum; }
    float GetFacingYaw() const { return mFacingYaw; }
    float GetRadius() const { return mRadius; }
    std::string GetModelPath() const { return mModelPath; }
    std::string GetName() const { return mName; }

    class TalkableComponent* GetTalkableComponent() const { return mTalkableComponent; }

    const std::vector<struct LoadedMesh>& GetMeshes() const { return mMeshes; }

private:
    void UpdateWorldVec();

private:
    bool mIsActive;

    int mCurrentPlanetNum;

    float mFacingYaw;
    float mRadius;

    glm::vec3 mPos;
    glm::vec3 mUpVec;
    glm::vec3 mForwardVec;
    glm::vec3 mLeftVec;
    glm::vec3 mFacingForwardVec;
    glm::vec3 mFacingLeftVec;

    std::string mModelPath;
    std::string mName;

    class Planet* mCurrentPlanet;

    class TalkableComponent* mTalkableComponent;

    std::vector<struct LoadedMesh> mMeshes;
};