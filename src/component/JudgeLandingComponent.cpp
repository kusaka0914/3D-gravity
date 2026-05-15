#include "JudgeLandingComponent.h"
#include "Game.h"
#include "actor/Actor.h"
#include "actor/Player.h"
#include "system/PhysicsSystem.h"
#include <btBulletDynamicsCommon.h>

JudgeLandingComponent::JudgeLandingComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
{
}

void JudgeLandingComponent::Update(float deltaTime) {
    CharacterActor* characterActor = dynamic_cast<CharacterActor*>(mOwner);
    if (characterActor->GetIsJudgeLanding())
        JudgeLanding();
}

void JudgeLandingComponent::JudgeLanding() {
    bool MeshGround = false;

    glm::vec3 ownerPos = mOwner->GetPos();
    glm::vec3 ownerUp = mOwner->GetUpVec();
    glm::vec3 rayFromPos = ownerPos + ownerUp * 0.1f;
    glm::vec3 rayToPos = ownerPos - ownerUp * 0.1f;
    btVector3 rayFrom(rayFromPos.x, rayFromPos.y, rayFromPos.z);
    btVector3 rayTo(rayToPos.x, rayToPos.y, rayToPos.z);

    btCollisionWorld::ClosestRayResultCallback rayCallback(rayFrom, rayTo);

    PhysicsSystem* physics = mOwner->GetGame()->GetPhysicsSystem();
    btDiscreteDynamicsWorld* bulletWorld = physics ? physics->GetBulletWorld() : nullptr;
    if (!bulletWorld) return;

    bulletWorld->rayTest(rayFrom, rayTo, rayCallback);

    CharacterActor* characterActor = dynamic_cast<CharacterActor*>(mOwner);

    if (rayCallback.hasHit())
    {
        btVector3 hitPt = rayCallback.m_hitPointWorld;
        glm::vec3 hitPos(hitPt.x(), hitPt.y(), hitPt.z());
    
        characterActor->OnLanded(hitPos);

        MeshGround = true;
    }

    bool onGround = characterActor->GetOnGround();
    if (!MeshGround && onGround)
    {
        characterActor->NotOnLanded();
    }
}