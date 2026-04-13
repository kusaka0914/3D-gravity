#ifndef BOAT_H
#define BOAT_H

#include "Actor.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class Boat : public Actor {
public:
    Boat(class Game* game);
    void UpdateActor(float deltaTime) override;

    void SetPlanets(std::vector<class Planet*> planets) { mPlanets = planets; }
    void SetCurrentPlanetNum(int currentPlanetNum) { mCurrentPlanetNum = currentPlanetNum; }
    void SetIsMoving(bool isMoving) { mIsMoving = isMoving; }
    void SetIsActive(bool isActive) { mIsActive = isActive; }
    void SetTransitionTimer(float transitionTimer) { mTransitionTimer = transitionTimer; }
    void SetProgress(float progress) { mProgress = progress; }
    void SetPos(const glm::vec3& pos) { mPos = pos; }
    void SetUpVec(const glm::vec3& upVec) { mUpVec = upVec; }
    void SetMeshes(const std::vector<struct LoadedMesh> meshes) { mMeshes = meshes; }

    int GetCurrentPlanetNum() const { return mCurrentPlanetNum; }
    int GetStartPlanet() const { return mStartPlanet; }
    int GetDestPlanet() const { return mDestPlanet; }
    bool GetIsMoving() const { return mIsMoving; }
    bool GetIsActive() const { return mIsActive; }
    float GetTransitionTimer() const { return mTransitionTimer; }
    float GetTransitionDuration() const { return mTransitionDuration; }
    float GetProgress() const { return mProgress; }
    const glm::vec3& GetPos() const { return mPos; }
    const glm::vec3& GetStartPos() const { return mStartPos; }
    const glm::vec3& GetDestPos() const { return mDestPos; }
    const glm::vec3& GetUpVec() const { return mUpVec; }
    const std::vector<class Planet*>& GetPlanets() const { return mPlanets; }
    const std::vector<struct LoadedMesh>& GetMeshes() const { return mMeshes; }

private:
    class Planet* mCurrentPlanet;
    int mCurrentPlanetNum;
    int mStartPlanet;
    int mDestPlanet;
    bool mIsMoving;
    bool mIsActive;
    float mTransitionTimer;
    float mTransitionDuration;
    float mProgress;
    glm::vec3 mPos;
    glm::vec3 mStartPos;
    glm::vec3 mDestPos;
    glm::vec3 mUpVec;
    std::vector<class Planet*> mPlanets;
    std::vector<struct LoadedMesh> mMeshes;
};
#endif
