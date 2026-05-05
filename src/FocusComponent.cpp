#include "Game.h"
#include "Player.h"
#include "FocusComponent.h"
#include "Boat.h"
#include "Key.h"
#include <glm/gtc/matrix_transform.hpp>

FocusComponent::FocusComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mFocusTimer(-1.0f)
{
}

void FocusComponent::Update(float deltaTime)
{
    if (mFocusTimer >= 0.0f) {
        mFocusTimer -= deltaTime;
    }
    if (mFocusTimer <= 2.0f && mFocusTimer >= 0.0f) {
        Boat* boat = dynamic_cast<Boat*>(GetOwner());
        Key* key = dynamic_cast<Key*>(GetOwner());
        if (boat) 
            boat->SetIsActive(true);
        if (key)
            key->SetIsActive(true);
    }
}

glm::mat4 FocusComponent::GetFocusView() {
    const float cameraDistance = 15.0f;
    Boat* boat = dynamic_cast<Boat*>(GetOwner());
    Key* key = dynamic_cast<Key*>(GetOwner());

    glm::vec3 pos;
    if (boat)
        pos = boat->GetPos();
    if (key)
        pos = key->GetPos();
        
    Planet* currentPlanet = GetOwner()->GetGame()->GetPlayers()[0]->GetCurrentPlanet();
    glm::vec3 upVec = glm::normalize(pos - currentPlanet->GetCenter());
    glm::vec3 worldLeft = glm::cross(upVec, glm::vec3(0, 0, 1));
    glm::vec3 forwardVec = glm::normalize(glm::cross(worldLeft, upVec) * std::cos(0.6f) - std::sin(0.6f) * worldLeft);
    glm::vec3 back = glm::normalize(-forwardVec);
    glm::vec3 cameraDir = glm::normalize(std::cos(-0.5f) * back + std::sin(-0.5f) * upVec);
    glm::vec3 cameraPos = pos - cameraDir * cameraDistance;
    return glm::lookAt(cameraPos, pos, upVec);
}