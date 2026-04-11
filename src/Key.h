#ifndef KEY_H
#define KEY_H

#include "Actor.h"
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

    bool GetIsActive() const { return mIsActive; }
    bool GetIsObtained() const { return mIsObtained; }
    int GetCurrentPlanet() const { return mCurrentPlanet; }

private:
    bool mIsActive;
    bool mIsObtained;
    int mCurrentPlanet;
    glm::vec3 mPos;
};
#endif
