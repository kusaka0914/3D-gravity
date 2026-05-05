#pragma once

#include "Actor.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class Boat : public Actor {
public:
    Boat(class Game* game);
    void Initialize() override;
    void UpdateActor(float deltaTime) override;

    void SetCurrentPlanet(class Planet* currentPlanet) { mCurrentPlanet = currentPlanet; }
    void SetDestPlanet(Planet* destPlanet) { mDestPlanet = destPlanet; }
    void SetDestStage(int destStage) { mDestStage = destStage; }
    void SetIsMoving(bool isMoving) { mIsMoving = isMoving; }
    void SetIsActive(bool isActive) { mIsActive = isActive; }
    void SetTransitionTimer(float transitionTimer) { mTransitionTimer = transitionTimer; }
    void SetProgress(float progress) { mProgress = progress; }
    void SetPos(const glm::vec3& pos) { mPos = pos; }
    void SetStartPos(const glm::vec3& startPos) { mStartPos = startPos; }
    void SetUpVec(const glm::vec3& upVec) { mUpVec = upVec; }
    void SetMeshes(std::vector<struct LoadedMesh>* meshes) { mMeshes = meshes; }

    bool GetIsMoving() const { return mIsMoving; }
    bool GetIsActive() const { return mIsActive; }
    float GetTransitionTimer() const { return mTransitionTimer; }
    float GetProgress() const { return mProgress; }
    const glm::vec3& GetPos() const { return mPos; }
    const glm::vec3& GetStartPos() const { return mStartPos; }
    const glm::vec3& GetDestPos() const { return mDestPos; }
    const glm::vec3& GetUpVec() const { return mUpVec; }
    const std::vector<struct LoadedMesh>* GetMeshes() const { return mMeshes; }
    class FocusComponent* GetFocusComponent() const { return mFocusComponent; }

private:
    void UpdateUpVec();
    void OnShown();
    void UpdateMoving(float deltaTime);
    void HandleMoving(float deltaTime);
    void HandleArrived();

private:
    class Planet* mCurrentPlanet;
    Planet* mDestPlanet;

    bool mIsMoving;
    bool mIsActivePrev;
    bool mIsActive;

    int mDestStage;
    
    float mTransitionTimer;
    float mProgress;

    glm::vec3 mPos;
    glm::vec3 mStartPos;
    glm::vec3 mDestPos;
    glm::vec3 mUpVec;

    std::vector<struct LoadedMesh>* mMeshes;
    class FocusComponent* mFocusComponent;
};