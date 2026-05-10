#include "Actor.h"
#include "Component.h"
#include "Game.h"
#include "Planet.h"
#include "PhysicsSystem.h"
#include <btBulletDynamicsCommon.h>

Actor::Actor(Game* game)
: mGame(game)
,mYaw(0.0f)
, mUpVec(0.0f, 1.0f, 0.0f)
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
    UpdateUpVec();
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
    glm::vec3 avgUpVec = GetAverageNormal();

    if (glm::length(avgUpVec) > 1e-6f) {
        mUpVec = avgUpVec;
    } else {
        UpdateFallbackUpVec();
    }
}

void Actor::UpdateFallbackUpVec() {
    auto planetShape = mCurrentPlanet->GetPlanetShape();
    auto normalShape = Planet::PlanetShape::Normal;

    if (planetShape == normalShape) {
        mUpVec = {0.0f, 1.0f, 0.0f};
    } else {
        glm::vec3 planetCenter = mCurrentPlanet->GetPos();
        mUpVec = glm::normalize(mPos - planetCenter);
    }
}

glm::vec3 Actor::GetAverageNormal()
{
    PhysicsSystem* physics = GetGame()->GetPhysicsSystem();
    btDiscreteDynamicsWorld* bulletWorld = physics ? physics->GetBulletWorld() : nullptr;
    if (!bulletWorld)
        return mUpVec;

    glm::vec3 up = glm::normalize(mUpVec);

    glm::vec3 side = glm::cross(up, glm::vec3(0.0f, 0.0f, 1.0f));
    if (glm::length(side) < 0.01f)
        side = glm::cross(up, glm::vec3(1.0f, 0.0f, 0.0f));
    side = glm::normalize(side);

    glm::vec3 forward = glm::normalize(glm::cross(side, up));

    const float footRadius = 0.25f;
    const float rayStartOffset = 0.2f;
    const float rayLength = 10.0f;
    const float minDot = 0.9f;

    auto castRay = [&](const glm::vec3& offset,
        glm::vec3& outNormal,
        const btCollisionObject*& outObj) -> bool
    {
    glm::vec3 fromPos = mPos + offset + up * rayStartOffset;
    glm::vec3 toPos   = mPos + offset - up * rayLength;

    btVector3 rayFrom(fromPos.x, fromPos.y, fromPos.z);
    btVector3 rayTo(toPos.x, toPos.y, toPos.z);

    btCollisionWorld::ClosestRayResultCallback cb(rayFrom, rayTo);
    bulletWorld->rayTest(rayFrom, rayTo, cb);

    if (!cb.hasHit())
    return false;

    btVector3 hitN = cb.m_hitNormalWorld;
    glm::vec3 hitNormal(hitN.x(), hitN.y(), hitN.z());
    if (glm::length(hitNormal) < 1e-6f)
    return false;

    hitNormal = glm::normalize(hitNormal);

    const float minDotAngle50 = 0.6428f;
    if (glm::dot(hitNormal, up) < minDotAngle50)
        return false;

    if (glm::dot(hitNormal, up) < 0.0f)
    hitNormal = -hitNormal;

    outNormal = hitNormal;
    outObj = cb.m_collisionObject;
    return true;
    };

    glm::vec3 mainNormal;
    const btCollisionObject* mainObj = nullptr;
    if (!castRay(glm::vec3(0.0f), mainNormal, mainObj))
        return glm::vec3(0.0f);
    
    mRayCastTimer = 0.5f;

    glm::vec3 normalSum = mainNormal * 3.0f;
    float weightSum = 3.0f;

    std::vector<glm::vec3> offsets = {
        forward * footRadius,
        -forward * footRadius,
        side * footRadius,
        -side * footRadius
    };

    for (const auto& offset : offsets) {
        glm::vec3 hitNormal;
        const btCollisionObject* hitObj = nullptr;
        if (!castRay(offset, hitNormal, hitObj))
            continue;

        if (hitObj != mainObj)
            continue;

        if (glm::dot(hitNormal, mainNormal) < minDot)
            continue;

        normalSum += hitNormal;
        weightSum += 1.0f;
    }

    return glm::normalize(normalSum / weightSum);
}

float Actor::getYawFromDirection(const glm::vec3& up, const glm::vec3& dir) {
    glm::vec3 worldLeft = glm::cross(mUpVec, glm::vec3(0, 0, 1));
    if (glm::length(worldLeft) < 0.01f){
        worldLeft = glm::normalize(glm::cross(mUpVec, glm::vec3(0, 1, 0)));
    }
    else 
        worldLeft = glm::normalize(worldLeft);
    glm::vec3 right = glm::cross(worldLeft, mUpVec);
    return std::atan2(-glm::dot(dir, worldLeft), glm::dot(dir, right));
}