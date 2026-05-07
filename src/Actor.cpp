#include "Actor.h"
#include "Component.h"
#include "Game.h"
#include "Planet.h"

Actor::Actor(Game* game)
: mGame(game)
,mYaw(0.0f)
{
    
}

Actor::~Actor() {
    
}

void Actor::Initialize() {
    
}

void Actor::ProcessInput() {
    ProcessActor();
}

void Actor::ProcessActor() {
    
}

void Actor::Update(float deltaTime) {
    UpdateActor(deltaTime);
    for (auto& component : mComponents) {
        Component* comp = component.get();
        comp->Update(deltaTime);
    }
}

void Actor::UpdateActor(float deltaTime) {
    
}

void Actor::AddComponent(std::unique_ptr<Component> component) {
    int myOrder = component->GetUpdateOrder();
    auto iter = mComponents.begin();
    for(;iter != mComponents.end();iter++) {
        if(myOrder < (*iter)->GetUpdateOrder()) {
            break;
        }
    }
    mComponents.insert(iter, std::move(component));
}

void Actor::RemoveComponent(std::unique_ptr<Component> component) {
    auto iter = std::find(mComponents.begin(), mComponents.end(), component);
    if (iter != mComponents.end()) {
        mComponents.erase(iter);
    }
}

void Actor::UpdateUpVec() {
    auto planetShape = mCurrentPlanet->GetPlanetShape();
    auto normalShape = Planet::PlanetShape::Normal;

    if (planetShape == normalShape) {
        mUpVec = {0.0f, 1.0f, 0.0f};
    } else {
        glm::vec3 planetCenter = mCurrentPlanet->GetPos();
        mUpVec = glm::normalize(mPos - planetCenter);
    }
}

float Actor::getYawFromDirection(const glm::vec3& up, const glm::vec3& dir) {
    glm::vec3 worldLeft = glm::normalize(glm::cross(mUpVec, glm::vec3(0, 0, 1)));
    if (glm::length(worldLeft) < 0.01f)
        worldLeft = glm::normalize(glm::cross(mUpVec, glm::vec3(1, 0, 0)));
    glm::vec3 right = glm::cross(worldLeft, mUpVec);
    return std::atan2(-glm::dot(dir, worldLeft), glm::dot(dir, right));
}