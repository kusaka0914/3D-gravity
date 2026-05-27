#include "actor/Actor.h"
#include "component/Component.h"
#include "Game.h"
#include "actor/Planet.h"
#include "actor/Player.h"
#include "system/PhysicsSystem.h"
#include <iostream>

Actor::Actor(Game* game)
: mGame(game)
, mUpVec(0.0f, 1.0f, 0.0f)
, mIsActive(true)
, mRadius(1.0f)
, mIsUpVecInitialized(false)
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
    CharacterActor* characterActor = dynamic_cast<CharacterActor*>(this);

    if (!characterActor && mIsUpVecInitialized) {
        return;
    }

    glm::vec3 avgUpVec = GetAverageNormal();

    if (!characterActor) {
        if (glm::length(avgUpVec) > 1e-6f) {
            mUpVec = avgUpVec;
        } else {
            UpdateFallbackUpVec();
        }

        mIsUpVecInitialized = true;
        return;
    }

    if (glm::length(avgUpVec) > 1e-6f) {
        mUpVec = avgUpVec;
        mIsUpVecInitialized = true;
        return;
    }

    Player* player = dynamic_cast<Player*>(this);
    if (!player) {
        UpdateFallbackUpVec();
        return;
    }

    if (player->GetRayCastTimer() <= 0.0f) {
        UpdateFallbackUpVec();
        player->SetVelocity(glm::vec3(0.0f));
        player->SetRayCastTimer(0.5f);
    }
}

void Actor::UpdateFallbackUpVec() {
    if (!mCurrentPlanet) {
        mUpVec = glm::vec3(0.0f, 1.0f, 0.0f);
        return;
    }

    if (mCurrentPlanet->GetPlanetShape() == Planet::PlanetShape::Normal) {
        mUpVec = glm::vec3(0.0f, 1.0f, 0.0f);
        return;
    }

    const glm::vec3 toActor = mPos - mCurrentPlanet->GetPos();

    if (glm::length(toActor) < 1e-6f) {
        mUpVec = glm::vec3(0.0f, 1.0f, 0.0f);
        return;
    }

    mUpVec = glm::normalize(toActor);
}

glm::vec3 Actor::GetAverageNormal() {
    glm::vec3 mainNormal;
    const btCollisionObject* mainObj = nullptr;
    if (!CastRay(glm::vec3(0.0f), mainNormal, mainObj)) {
        return glm::vec3(0.0f);
    }

    constexpr float mainWeight = 3.0f;
    glm::vec3 normalSum = mainNormal * mainWeight;
    float weightSum = mainWeight;

    glm::vec3 up = glm::normalize(mUpVec);

    glm::vec3 side = glm::cross(up, glm::vec3(0.0f, 0.0f, 1.0f));
    if (glm::length(side) < 0.01f)
        side = glm::cross(up, glm::vec3(1.0f, 0.0f, 0.0f));
    side = glm::normalize(side);

    glm::vec3 forward = glm::normalize(glm::cross(side, up));
    constexpr float footRadius = 0.25f;
    std::vector<glm::vec3> offsets = {
        forward * footRadius,
        -forward * footRadius,
        side * footRadius,
        -side * footRadius
    };

    for (const auto& offset : offsets) {
        glm::vec3 hitNormal;
        const btCollisionObject* hitObj = nullptr;
        if (!CastRay(offset, hitNormal, hitObj))
            continue;

        if (hitObj != mainObj)
            continue;

        normalSum += hitNormal;
        weightSum += 1.0f;
    }

    const glm::vec3 averageNormal = normalSum / weightSum;

    if (glm::length(averageNormal) < 1e-6f) {
        return glm::vec3(0.0f);
    }

    return glm::normalize(averageNormal);
}

bool Actor::CastRay(const glm::vec3& offset, glm::vec3& outNormal, const btCollisionObject*& outObj) {  
    if (glm::length(mUpVec) < 1e-6f) {
        UpdateFallbackUpVec();
    }

    if (glm::length(mUpVec) < 1e-6f) {
        return false;
    }

    const glm::vec3 up = glm::normalize(mUpVec);
    constexpr float rayStartOffset = 0.2f;
    constexpr float rayLength = 30.0f;

    glm::vec3 downRayFromPos = mPos + offset + up * rayStartOffset;
    glm::vec3 downRayToPos   = mPos + offset - up * rayLength;

    // glm::vec3 upRayFromPos = mPos + offset + up * rayStartOffset;
    // glm::vec3 upRayToPos   = mPos + offset + up * rayLength;

    btVector3 downRayFrom(downRayFromPos.x, downRayFromPos.y, downRayFromPos.z);
    btVector3 downRayTo(downRayToPos.x, downRayToPos.y, downRayToPos.z);

    // btVector3 upRayFrom(upRayFromPos.x, upRayFromPos.y, upRayFromPos.z);
    // btVector3 upRayTo(upRayToPos.x, upRayToPos.y, upRayToPos.z);

    btCollisionWorld::ClosestRayResultCallback cb(downRayFrom, downRayTo);
    // btCollisionWorld::ClosestRayResultCallback cb2(upRayFrom, upRayTo);

    btDiscreteDynamicsWorld* bulletWorld = mGame->GetPhysicsSystem()->GetBulletWorld();

    bulletWorld->rayTest(downRayFrom, downRayTo, cb);
    // bulletWorld->rayTest(upRayFrom, upRayTo, cb2);

    if (!cb.hasHit()
    // && !cb2.hasHit()
    ) {
        return false;
    }

    btVector3 hitN;
    const btCollisionObject* chosenObj = nullptr;

    if (cb.hasHit()
    // && !cb2.hasHit()
    ) {
        hitN = cb.m_hitNormalWorld;
        chosenObj = cb.m_collisionObject;
    }
    // else if (!cb.hasHit() && cb2.hasHit()) {
    //     hitN = cb2.m_hitNormalWorld;
    //     chosenObj = cb2.m_collisionObject;
    // }
    // else {
    //     glm::vec3 hitPos1(cb.m_hitPointWorld.x(), cb.m_hitPointWorld.y(), cb.m_hitPointWorld.z());
    //     glm::vec3 hitPos2(cb2.m_hitPointWorld.x(), cb2.m_hitPointWorld.y(), cb2.m_hitPointWorld.z());

    //     float dist1 = glm::length(mPos - hitPos1);
    //     float dist2 = glm::length(mPos - hitPos2);

    //     if (dist1 <= dist2) {
    //         hitN = cb.m_hitNormalWorld;
    //         chosenObj = cb.m_collisionObject;
    //     } else {
    //         hitN = cb2.m_hitNormalWorld;
    //         chosenObj = cb2.m_collisionObject;
    //     }
    // }

    glm::vec3 hitNormal(hitN.x(), hitN.y(), hitN.z());
    if (glm::length(hitNormal) < 1e-6f)
        return false;

    hitNormal = glm::normalize(hitNormal);

    Player* player = dynamic_cast<Player*>(this);
    if (player) {
        player->SetRayCastTimer(0.5f);
    }

    CharacterActor* characterActor = dynamic_cast<CharacterActor*>(this);
    if (characterActor) {
        const float minDotAngle50 = 0.6428f;
        const float minDotAngleMinus50 = -0.6428f;
        if (glm::dot(hitNormal, up) < minDotAngle50 && glm::dot(hitNormal, up) > minDotAngleMinus50)
            return false;
    }

    outNormal = hitNormal;
    outObj = chosenObj;
    return true;
}