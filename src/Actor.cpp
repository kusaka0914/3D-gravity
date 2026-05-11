#include "Actor.h"
#include "Component.h"
#include "Game.h"
#include "Planet.h"
#include "Player.h"
#include "PhysicsSystem.h"
#include <btBulletDynamicsCommon.h>

Actor::Actor(Game* game)
: mGame(game)
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
        return;
    }
    
    Player* player = dynamic_cast<Player*>(this);
    if (!player){
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
    auto planetShape = mCurrentPlanet->GetPlanetShape();
    auto normalShape = Planet::PlanetShape::Normal;

    if (planetShape == normalShape) {
        mUpVec = {0.0f, 1.0f, 0.0f};
        return;
    }

    glm::vec3 planetCenter = mCurrentPlanet->GetPos();
    mUpVec = glm::normalize(mPos - planetCenter);
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
    const float rayLength = 30.0f;
    const float minDot = 0.9f;

    auto castRay = [&](const glm::vec3& offset,
        glm::vec3& outNormal,
        const btCollisionObject*& outObj) -> bool
        {
        glm::vec3 downRayFromPos = mPos + offset + up * rayStartOffset;
        glm::vec3 downRayToPos   = mPos + offset - up * rayLength;

        glm::vec3 upRayFromPos = mPos + offset + up * rayStartOffset;
        glm::vec3 upRayToPos   = mPos + offset + up * rayLength;

        btVector3 downRayFrom(downRayFromPos.x, downRayFromPos.y, downRayFromPos.z);
        btVector3 downRayTo(downRayToPos.x, downRayToPos.y, downRayToPos.z);

        btVector3 upRayFrom(upRayFromPos.x, upRayFromPos.y, upRayFromPos.z);
        btVector3 upRayTo(upRayToPos.x, upRayToPos.y, upRayToPos.z);

        btCollisionWorld::ClosestRayResultCallback cb(downRayFrom, downRayTo);
        btCollisionWorld::ClosestRayResultCallback cb2(upRayFrom, upRayTo);

        bulletWorld->rayTest(downRayFrom, downRayTo, cb);
        bulletWorld->rayTest(upRayFrom, upRayTo, cb2);

        if (!cb.hasHit() && !cb2.hasHit())
        return false;

        btVector3 hitN;
        const btCollisionObject* chosenObj = nullptr;

        if (cb.hasHit() && !cb2.hasHit()) {
        hitN = cb.m_hitNormalWorld;
        chosenObj = cb.m_collisionObject;
        }
        else if (!cb.hasHit() && cb2.hasHit()) {
        hitN = cb2.m_hitNormalWorld;
        chosenObj = cb2.m_collisionObject;
        }
        else {
        glm::vec3 hitPos1(cb.m_hitPointWorld.x(), cb.m_hitPointWorld.y(), cb.m_hitPointWorld.z());
        glm::vec3 hitPos2(cb2.m_hitPointWorld.x(), cb2.m_hitPointWorld.y(), cb2.m_hitPointWorld.z());

        float dist1 = glm::length(mPos - hitPos1);
        float dist2 = glm::length(mPos - hitPos2);

        if (dist1 <= dist2) {
        hitN = cb.m_hitNormalWorld;
        chosenObj = cb.m_collisionObject;
        } else {
        hitN = cb2.m_hitNormalWorld;
        chosenObj = cb2.m_collisionObject;
        }
        }

        glm::vec3 hitNormal(hitN.x(), hitN.y(), hitN.z());
        if (glm::length(hitNormal) < 1e-6f)
        return false;

        hitNormal = glm::normalize(hitNormal);

        Player* player = dynamic_cast<Player*>(this);
        if (player) {
            player->SetRayCastTimer(0.5f);
            const float minDotAngle50 = 0.6428f;
            if (glm::dot(hitNormal, up) < minDotAngle50)
                return false;
        }

        outNormal = hitNormal;
        outObj = chosenObj;
        return true;
        };

    glm::vec3 mainNormal;
    const btCollisionObject* mainObj = nullptr;
    if (!castRay(glm::vec3(0.0f), mainNormal, mainObj))
        return glm::vec3(0.0f);

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

        // if (glm::dot(hitNormal, mainNormal) < minDot)
        //     continue;

        normalSum += hitNormal;
        weightSum += 1.0f;
    }

    return glm::normalize(normalSum / weightSum);
}