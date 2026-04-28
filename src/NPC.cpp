#include "NPC.h"
#include "Enemy.h"
#include "Boat.h"
#include "Key.h"
#include "Stage.h"
#include "Star.h"
#include "Game.h"
#include "PhysicsSystem.h"
#include "UIState.h"
#include "GameProgressState.h"
#include <btBulletDynamicsCommon.h>
#include <cmath>

NPC::NPC(Game* game)
    : Actor(game)
    , mCurrentPlanet(nullptr)
    , mIsActive(true)
    , mPos(0.0f)
    , mUpVec(0.0f)
    , mCurrentPlanetNum(0)
    , mFacingYaw(0.0f)
{
    
}

NPC::~NPC()
{
    
}

void NPC::ProcessActor()
{   
    
}

void NPC::UpdateActor(float deltaTime)
{
    UpdateWorldVec();
}

void NPC::UpdateWorldVec() {
    if (mCurrentPlanet->GetPlanetType() == Planet::PlanetType::Normal) {
        mUpVec = {0.0f, 1.0f, 0.0f};
    } else {
        mUpVec = glm::normalize(mPos - mCurrentPlanet->GetCenter());
    }

    glm::vec3 worldLeft = glm::cross(mUpVec, glm::vec3(0, 0, 1));
    if (glm::length(worldLeft) < 0.01f)
        worldLeft = glm::normalize(glm::cross(mUpVec, glm::vec3(0, 1, 0)));
    else 
        worldLeft = glm::normalize(worldLeft);

    // mForwardVec = glm::normalize(glm::cross(worldLeft, mUpVec) * std::cos(mCameraYaw) - std::sin(mCameraYaw) * worldLeft);
    // mLeftVec = glm::normalize(glm::cross(mUpVec, mForwardVec));

    mFacingForwardVec = glm::normalize(glm::cross(worldLeft, mUpVec) * std::cos(mFacingYaw) - std::sin(mFacingYaw) * worldLeft);
    mFacingLeftVec = glm::normalize(glm::cross(mUpVec, mFacingForwardVec));
}

float NPC::getYawFromDirection(const glm::vec3& mUpVec, const glm::vec3& dir) {
    glm::vec3 worldLeft = glm::normalize(glm::cross(mUpVec, glm::vec3(0, 0, 1)));
    if (glm::length(worldLeft) < 0.01f)
        worldLeft = glm::normalize(glm::cross(mUpVec, glm::vec3(1, 0, 0)));
    glm::vec3 right = glm::cross(worldLeft, mUpVec);
    return std::atan2(-glm::dot(dir, worldLeft), glm::dot(dir, right));
}

glm::mat4 NPC::getNPCView(float cameraDistance, bool isFixed) {
    glm::vec3 toPosX;
    glm::vec3 cameraDir;
    glm::vec3 cameraPos;
    if (isFixed) {
        toPosX = glm::normalize(mFacingForwardVec);
        cameraDir = glm::normalize(std::cos(-0.2f) * toPosX + std::sin(-0.2f) * mUpVec);  
        cameraPos = mPos - cameraDir * cameraDistance;
        glm::vec3 offset = glm::normalize(mUpVec) * 1.0f;
        return glm::lookAt(cameraPos, mPos + offset, mUpVec);  
    }

    // toPosX = glm::normalize(-mForwardVec);
    // cameraDir = glm::normalize(std::cos(mCameraPitch) * toPosX + std::sin(mCameraPitch) * mUpVec);
    // cameraPos = mPos - cameraDir * cameraDistance;
    // return glm::lookAt(cameraPos, mPos, mUpVec);
}