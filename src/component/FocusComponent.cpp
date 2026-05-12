#include "Game.h"
#include "FocusComponent.h"
#include "actor/Player.h"
#include <glm/gtc/matrix_transform.hpp>

FocusComponent::FocusComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mFocusTimer(-1.0f)
    , mIsFocused(false)
{
}

void FocusComponent::Update(float deltaTime)
{
    if (mIsFocused && mFocusTimer <= 2.0f && mFocusTimer >= 0.0f)
        mOwner->SetIsActive(true);

    if (mFocusTimer >= 0.0f) 
        mFocusTimer -= deltaTime;
}

glm::mat4 FocusComponent::GetFocusView() {
    const float cameraDistance = 15.0f;
    glm::vec3 ownerPos = mOwner->GetPos();

    Planet* currentPlanet = mOwner->GetGame()->GetPlayers()[0]->GetCurrentPlanet();
    glm::vec3 upVec = glm::normalize(ownerPos - currentPlanet->GetPos());
    glm::vec3 worldLeft = glm::cross(upVec, glm::vec3(0, 0, 1));
    if (glm::length(worldLeft) < 0.01f){
        worldLeft = glm::normalize(glm::cross(upVec, glm::vec3(0, 1, 0)));
    }
    else 
        worldLeft = glm::normalize(worldLeft);
    glm::vec3 forwardVec = glm::normalize(glm::cross(worldLeft, upVec) * std::cos(0.6f) - std::sin(0.6f) * worldLeft);
    glm::vec3 back = glm::normalize(-forwardVec);
    glm::vec3 cameraDir = glm::normalize(std::cos(-0.5f) * back + std::sin(-0.5f) * upVec);
    glm::vec3 cameraPos = ownerPos - cameraDir * cameraDistance;
    return glm::lookAt(cameraPos, ownerPos, upVec);
}