#include "CameraSystem.h"
#include "Game.h"
#include "actor/Player.h"
#include <iostream>

CameraSystem::CameraSystem(Game* game) 
    :mGame(game)
    , mCameraPitch(0.4f)
    , mCameraYaw(0.0f)
{

}

void CameraSystem::ProcessInput() {
    SDL_GameController* sdlController = mGame->GetSdlController();
    constexpr float deadZone = 0.25f;
    constexpr float scale = 1.0f / 32767.0f; // SDL_GameControllerGetAxisの範囲が32767までで、scaleをかけて1.0f以内に抑えるため
    // 右スティックの操作をカメラ移動量に
    mCameraStickY = SDL_GameControllerGetAxis(sdlController, SDL_CONTROLLER_AXIS_RIGHTY) * scale;
    mCameraStickX = SDL_GameControllerGetAxis(sdlController, SDL_CONTROLLER_AXIS_RIGHTX) * scale;

    if (std::abs(mCameraStickY) < deadZone)
        mCameraStickY = 0.0f;
    if (std::abs(mCameraStickX) < deadZone)
        mCameraStickX = 0.0f;
}

void CameraSystem::Update(float deltaTime) {
    UpdateCamera(deltaTime);
}

void CameraSystem::UpdateCamera(float deltaTime) {
    constexpr float cameraSensitivity = 2.5f;

    mCameraYaw = mCameraStickX * cameraSensitivity * deltaTime;
    mCameraPitch -= mCameraStickY * cameraSensitivity * deltaTime;
    mCameraPitch = glm::clamp(mCameraPitch, -1.2f, -0.2f);

    Player* player = mGame->GetPlayers()[0];
    const float upSmooth = 1.0f - std::exp(-8.0f * deltaTime);
    mCameraUpVec = glm::normalize(glm::mix(mCameraUpVec, player->GetUpVec(), upSmooth));

    const float targetSmooth = 1.0f - std::exp(-10.0f * deltaTime);
    mCameraTargetPos = glm::mix(mCameraTargetPos, player->GetPos(), targetSmooth);
}

glm::mat4 CameraSystem::GetPlayerView(float cameraDistance, bool isFixed) {
    glm::vec3 toPosX;
    glm::vec3 cameraDir;
    Player* player = mGame->GetPlayers()[0];

    if (isFixed) {
        glm::vec3 facingForwardVec = player->GetFacingForwardVec();
        toPosX = glm::normalize(-facingForwardVec);
        cameraDir = glm::normalize(std::cos(-0.2f) * toPosX + std::sin(-0.2f) * mCameraUpVec);
        mCameraPos = mCameraTargetPos - cameraDir * cameraDistance;
        glm::vec3 offset = glm::normalize(mCameraUpVec) * 1.0f;
        return glm::lookAt(mCameraPos, mCameraTargetPos + offset, mCameraUpVec);
    }

    glm::vec3 forwardVec = player->GetForwardVec();
    toPosX = glm::normalize(-forwardVec);
    cameraDir = glm::normalize(std::cos(mCameraPitch) * toPosX + std::sin(mCameraPitch) * mCameraUpVec);
    mCameraPos = mCameraTargetPos - cameraDir * cameraDistance;
    return glm::lookAt(mCameraPos, mCameraTargetPos, mCameraUpVec);
}

// glm::mat4 CameraSystem::GetView(Actor* actor, float cameraDistance, bool isFixed) {
//     glm::vec3 toPosX;
//     glm::vec3 cameraDir;

//     if (isFixed) {
//         toPosX = glm::normalize(-mFacingForwardVec);
//         cameraDir = glm::normalize(std::cos(-0.2f) * toPosX + std::sin(-0.2f) * mCameraUpVec);
//         mCameraPos = mCameraTargetPos - cameraDir * cameraDistance;
//         glm::vec3 offset = glm::normalize(mCameraUpVec) * 1.0f;
//         return glm::lookAt(mCameraPos, mCameraTargetPos + offset, mCameraUpVec);
//     }

//     toPosX = glm::normalize(-mForwardVec);
//     cameraDir = glm::normalize(std::cos(mCameraPitch) * toPosX + std::sin(mCameraPitch) * mCameraUpVec);
//     mCameraPos = mCameraTargetPos - cameraDir * cameraDistance;
//     return glm::lookAt(mCameraPos, mCameraTargetPos, mCameraUpVec);
// }

// glm::mat4 FocusComponent::GetFocusView() {
//     const float cameraDistance = 15.0f;
//     glm::vec3 ownerPos = mOwner->GetPos();

//     Planet* currentPlanet = mOwner->GetGame()->GetPlayers()[0]->GetCurrentPlanet();
//     glm::vec3 upVec = glm::normalize(ownerPos - currentPlanet->GetPos());
//     glm::vec3 worldLeft = glm::cross(upVec, glm::vec3(0, 0, 1));
//     if (glm::length(worldLeft) < 0.01f){
//         worldLeft = glm::normalize(glm::cross(upVec, glm::vec3(0, 1, 0)));
//     }
//     else 
//         worldLeft = glm::normalize(worldLeft);
//     glm::vec3 forwardVec = glm::normalize(glm::cross(worldLeft, upVec) * std::cos(0.6f) - std::sin(0.6f) * worldLeft);
//     glm::vec3 back = glm::normalize(-forwardVec);
//     glm::vec3 cameraDir = glm::normalize(std::cos(-0.5f) * back + std::sin(-0.5f) * upVec);
//     glm::vec3 cameraPos = ownerPos - cameraDir * cameraDistance;
//     return glm::lookAt(cameraPos, ownerPos, upVec);
// }