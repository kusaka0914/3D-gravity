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
    void SetCurrentPlanetNum(int currentPlanetNum) { mCurrentPlanetNum = currentPlanetNum; }
    void SetFacingYaw(float facingYaw) { mFacingYaw = facingYaw; }
    void SetRadius(float radius) { mRadius = radius; }
    void SetModelPath(std::string modelPath) { mModelPath = modelPath; }
    void SetName(std::string name) { mName = name; }
    void SetTalkableComponent(class TalkableComponent* talkableComponent) { mTalkableComponent = talkableComponent; }
    void SetMeshes(std::vector<struct LoadedMesh>* meshes) { mMeshes = meshes; }

    bool GetIsActive() const { return mIsActive; }
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

    std::vector<struct LoadedMesh>* GetMeshes() const { return mMeshes; }

private:
    void UpdateWorldVec();

private:
    bool mIsActive;

    int mCurrentPlanetNum;

    float mFacingYaw;
    float mRadius;

    glm::vec3 mForwardVec;
    glm::vec3 mLeftVec;
    glm::vec3 mFacingForwardVec;
    glm::vec3 mFacingLeftVec;

    std::string mModelPath;
    std::string mName;

    class TalkableComponent* mTalkableComponent;

    std::vector<struct LoadedMesh>* mMeshes;
};