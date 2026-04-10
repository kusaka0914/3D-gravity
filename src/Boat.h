#ifndef BOART_H
#define BOART_H

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class Boat : Actor {
public:
    Boat(class Game* game);
    void UpdateActor(float deltaTime) override;

    void SetCurrentPlanet(int currentPlanet) { mCurrentPlanet = currentPlanet; }
    void SetTransitionTimer(float transitionTimer) { mTransitionTimer = transitionTimer; }
    void SetProgress(float progress) { mProgress = progress; }
    void SetPos(const glm::vec3& pos) { mPos = pos; }
    void SetUpVec(const glm::vec3& upVec) { mUpVec = upVec; }

    int GetCurrentPlanet() const { return mCurrentPlanet; }
    int GetStartPlanet() const { return mStartPlanet; }
    int GetDestPlanet() const { return mDestPlanet; }
    bool GetIsActive() const { return mIsActive; }
    float GetTransitionTimer() const { return mTransitionTimer; }
    float GetTransitionDuration() const { return mTransitionDuration; }
    float GetProgress() const { return mProgress; }
    const glm::vec3& GetPos() const { return mPos; }
    const glm::vec3& GetStartPos() const { return mStartPos; }
    const glm::vec3& GetDestPos() const { return mDestPos; }
    const glm::vec3& GetUpVec() const { return mUpVec; }

private:
    int mCurrentPlanet;
    int mStartPlanet;
    int mDestPlanet;
    bool mIsActive;
    float mTransitionTimer;
    float mTransitionDuration;
    float mProgress;
    glm::vec3 mPos;
    glm::vec3 mStartPos;
    glm::vec3 mDestPos;
    glm::vec3 mUpVec;
};
#endif
