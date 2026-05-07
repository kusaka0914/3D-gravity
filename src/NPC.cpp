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
#include "TalkableComponent.h"
#include <btBulletDynamicsCommon.h>
#include <cmath>

NPC::NPC(Game* game)
    : Actor(game)
    , mIsActive(true)
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
    UpdateUpVec();
    UpdateWorldVec();
}

void NPC::UpdateWorldVec() {
    glm::vec3 worldLeft = glm::cross(GetUpVec(), glm::vec3(0, 0, 1));
    if (glm::length(worldLeft) < 0.01f)
        worldLeft = glm::normalize(glm::cross(GetUpVec(), glm::vec3(0, 1, 0)));
    else 
        worldLeft = glm::normalize(worldLeft);

    // mForwardVec = glm::normalize(glm::cross(worldLeft, GetUpVec()) * std::cos(mCameraYaw) - std::sin(mCameraYaw) * worldLeft);
    // mLeftVec = glm::normalize(glm::cross(GetUpVec(), mForwardVec));

    mFacingForwardVec = glm::normalize(glm::cross(worldLeft, GetUpVec()) * std::cos(mFacingYaw) - std::sin(mFacingYaw) * worldLeft);
    mFacingLeftVec = glm::normalize(glm::cross(GetUpVec(), mFacingForwardVec));
}

float NPC::getYawFromDirection(const glm::vec3& up, const glm::vec3& dir) {
    glm::vec3 worldLeft = glm::normalize(glm::cross(GetUpVec(), glm::vec3(0, 0, 1)));
    if (glm::length(worldLeft) < 0.01f)
        worldLeft = glm::normalize(glm::cross(GetUpVec(), glm::vec3(1, 0, 0)));
    glm::vec3 right = glm::cross(worldLeft, GetUpVec());
    return std::atan2(-glm::dot(dir, worldLeft), glm::dot(dir, right));
}

glm::mat4 NPC::getNPCView(float cameraDistance, bool isFixed) {
    glm::vec3 toPosX;
    glm::vec3 cameraDir;
    glm::vec3 cameraPos;
    if (isFixed) {
        toPosX = glm::normalize(mFacingForwardVec);
        cameraDir = glm::normalize(std::cos(-0.2f) * toPosX + std::sin(-0.2f) * GetUpVec());  
        cameraPos = mPos - cameraDir * cameraDistance;
        glm::vec3 offset = glm::normalize(GetUpVec()) * 1.0f;
        return glm::lookAt(cameraPos, mPos + offset, GetUpVec());  
    }

    // toPosX = glm::normalize(-mForwardVec);
    // cameraDir = glm::normalize(std::cos(mCameraPitch) * toPosX + std::sin(mCameraPitch) * GetUpVec());
    // cameraPos = mPos - cameraDir * cameraDistance;
    // return glm::lookAt(cameraPos, mPos, GetUpVec());
}