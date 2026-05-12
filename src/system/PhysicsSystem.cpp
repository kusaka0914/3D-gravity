#include "PhysicsSystem.h"
#include "Game.h"
#include "system/Mesh.h"
#include "Stage.h"
#include "actor/Planet.h"
#include "actor/Player.h"
#include "actor/Enemy.h"
#include "actor/Crystal.h"
#include "actor/NPC.h"
#include "actor/Platform.h"
#include "actor/Actor.h"

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>

#include <iostream>
#include <string>

PhysicsSystem::PhysicsSystem(Game* game)
    : mGame(game)
    , mBulletInitialized(false)
    , mMeshLoader(nullptr)
{
    Initialize();
}

PhysicsSystem::~PhysicsSystem()
{
    ClearBulletWorld();
}

void PhysicsSystem::ClearBulletWorld()
{
    if (mBulletWorld)
    {
        if (mBulletCharController)
        {
            mBulletWorld->removeAction(mBulletCharController.get());
        }

        if (mBulletGhost)
        {
            mBulletWorld->removeCollisionObject(mBulletGhost.get());
        }

        for (const auto& body : mBulletPlanetBodies)
        {
            if (body)
            {
                mBulletWorld->removeRigidBody(body.get());
            }
        }
    }

    // world から外した後に所有物を破棄
    mBulletCharController.reset();
    mBulletGhost.reset();

    mBulletWallSphere.reset();
    mBulletCapsule.reset();

    mBulletPlanetBodies.clear();
    mBulletPlanetShapes.clear();
    mBulletPlanetMeshes.clear();

    // world は依存先より先に破棄する
    mBulletWorld.reset();

    mBulletSolver.reset();
    mBulletBroadphase.reset();

    mBulletGhostPairCallback.reset();

    mBulletDispatcher.reset();
    mBulletCollisionConfig.reset();

    mBulletInitialized = false;
}

void PhysicsSystem::Initialize()
{
    ClearBulletWorld();

    mMeshLoader = mGame->GetMesh();

    const auto planets = mGame->GetCurrentStage()->GetPlanets();
    if (planets.empty())
    {
        std::cerr << "Bullet: planet mesh load failed" << std::endl;
        mBulletInitialized = false;
        return;
    }

    mBulletCollisionConfig =
        std::make_unique<btDefaultCollisionConfiguration>();

    mBulletDispatcher =
        std::make_unique<btCollisionDispatcher>(
            mBulletCollisionConfig.get());

    mBulletBroadphase =
        std::make_unique<btDbvtBroadphase>();

    mBulletSolver =
        std::make_unique<btSequentialImpulseConstraintSolver>();

    mBulletWorld =
        std::make_unique<btDiscreteDynamicsWorld>(
            mBulletDispatcher.get(),
            mBulletBroadphase.get(),
            mBulletSolver.get(),
            mBulletCollisionConfig.get());

    mBulletWorld->setGravity(btVector3(0, -9.8f, 0));

    mBulletGhostPairCallback =
        std::make_unique<btGhostPairCallback>();

    mBulletBroadphase->getOverlappingPairCache()
        ->setInternalGhostPairCallback(mBulletGhostPairCallback.get());

    CreateWorld();
}

void PhysicsSystem::CreateWorld() {
    CreateStageCollisionBodies();

    if (!CreatePlayerCharacterController()) return;

    mBulletInitialized = true;
}

void PhysicsSystem::CreateStageCollisionBodies() {
    const std::vector<Planet*> planets = mGame->GetCurrentStage()->GetPlanets();
    for (auto planet : planets)
    {
        CreateBody(planet);

        std::vector<Platform*> platforms = planet->GetPlatforms();
        for (auto platform : platforms) {
            CreateBody(platform);
        }

        // TODO: クリスタルも当たり判定に
        // std::vector<Crystal*> crystals = planet->GetCrystals;
        // for (auto crystal : crystals) {
        //     CreateBody(crystal);
        // }
    }
}

void PhysicsSystem::CreateBody(Actor* actor)
{
    std::string meshPath = "../assets/models/" + actor->GetModelPath();

    std::vector<float> pos;
    std::vector<unsigned int> idx;

    if (!mMeshLoader->loadMeshPositionsAndIndices(
            meshPath.c_str(), pos, idx)
        || pos.size() < 9
        || idx.size() < 3)
    {
        return;
    }

    const glm::vec3& actorPos = actor->GetPos();
    const glm::vec3& scale = actor->GetScale();

    auto triMesh = std::make_unique<btTriangleMesh>();

    for (size_t i = 0; i + 2 < idx.size(); i += 3)
    {
        const unsigned int i0 = idx[i];
        const unsigned int i1 = idx[i + 1];
        const unsigned int i2 = idx[i + 2];

        if (i0 * 3 + 2 >= pos.size()
            || i1 * 3 + 2 >= pos.size()
            || i2 * 3 + 2 >= pos.size())
        {
            return;
        }

        const btVector3 v0(
            actorPos.x + scale.x * pos[i0 * 3],
            actorPos.y + scale.y * pos[i0 * 3 + 1],
            actorPos.z + scale.z * pos[i0 * 3 + 2]);

        const btVector3 v1(
            actorPos.x + scale.x * pos[i1 * 3],
            actorPos.y + scale.y * pos[i1 * 3 + 1],
            actorPos.z + scale.z * pos[i1 * 3 + 2]);

        const btVector3 v2(
            actorPos.x + scale.x * pos[i2 * 3],
            actorPos.y + scale.y * pos[i2 * 3 + 1],
            actorPos.z + scale.z * pos[i2 * 3 + 2]);

        triMesh->addTriangle(v0, v1, v2);
    }

    auto shape =
        std::make_unique<btBvhTriangleMeshShape>(
            triMesh.get(),
            true);

    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(btVector3(0, 0, 0));

    btRigidBody::btRigidBodyConstructionInfo rbInfo(
        0,
        nullptr,
        shape.get());

    auto body = std::make_unique<btRigidBody>(rbInfo);

    body->setWorldTransform(startTransform);
    body->setCollisionFlags(
        body->getCollisionFlags()
        | btCollisionObject::CF_STATIC_OBJECT);

    mBulletPlanetMeshes.emplace_back(std::move(triMesh));
    mBulletPlanetShapes.emplace_back(std::move(shape));
    mBulletPlanetBodies.emplace_back(std::move(body));

    mBulletWorld->addRigidBody(
        mBulletPlanetBodies.back().get(),
        static_cast<short>(btBroadphaseProxy::DefaultFilter),
        static_cast<short>(-1));
}

bool PhysicsSystem::CreatePlayerCharacterController()
{
    const auto& players = mGame->GetPlayers();
    if (players.empty())
    {
        return false;
    }

    const std::vector<Planet*> planets =
        mGame->GetCurrentStage()->GetPlanets();

    if (planets.empty())
    {
        return false;
    }

    constexpr float capRadius = 0.35f;
    constexpr float capHeight = 0.8f;
    constexpr float stepHeight = 0.35f;

    mBulletCapsule =
        std::make_unique<btCapsuleShape>(
            capRadius,
            capHeight);

    mBulletWallSphere =
        std::make_unique<btSphereShape>(
            capRadius);

    mBulletGhost =
        std::make_unique<btPairCachingGhostObject>();

    mBulletGhost->setCollisionShape(mBulletCapsule.get());
    mBulletGhost->setCollisionFlags(
        btCollisionObject::CF_CHARACTER_OBJECT);

    const glm::vec3 spawnUp =
        glm::normalize(
            players[0]->GetPos() - planets[0]->GetPos());

    const float capHalf = capHeight * 0.5f;

    const glm::vec3 ghostOrigin =
        players[0]->GetPos()
        + spawnUp * (capHalf + 0.15f);

    btTransform ghostTrans;
    ghostTrans.setIdentity();
    ghostTrans.setOrigin(
        btVector3(
            ghostOrigin.x,
            ghostOrigin.y,
            ghostOrigin.z));

    mBulletGhost->setWorldTransform(ghostTrans);

    mBulletWorld->addCollisionObject(
        mBulletGhost.get(),
        static_cast<short>(btBroadphaseProxy::CharacterFilter),
        static_cast<short>(-1));

    mBulletCharController =
        std::make_unique<btKinematicCharacterController>(
            mBulletGhost.get(),
            mBulletCapsule.get(),
            stepHeight);

    mBulletCharController->setGravity(btVector3(0, -9.8f, 0));
    mBulletCharController->setJumpSpeed(5.0f);
    mBulletCharController->setFallSpeed(55.0f);

    mBulletWorld->addAction(mBulletCharController.get());

    return true;
}

void PhysicsSystem::Update()
{
    if (!mBulletWorld) return;

    constexpr btScalar fixedTimeStep = btScalar(1.0f / 60.0f);
    constexpr int maxSubSteps = 1;
    mBulletWorld->stepSimulation(fixedTimeStep, maxSubSteps);
}

glm::vec3 PhysicsSystem::CheckCollision(glm::vec3 moveDelta, glm::vec3 desiredPos) {
    std::vector<Player*> players = mGame->GetPlayers();
    std::vector<Enemy*> enemies = players[0]->GetCurrentPlanet()->GetEnemies();
    // 敵との当たり判定
    for (auto* enemy : enemies)
    {
        if (!enemy->GetIsAlive())
            continue;
        glm::vec3 ePos = enemy->GetPos();
        glm::vec3 toDesired = desiredPos - ePos;
        float d = glm::length(toDesired);
        float minDist = enemy->GetRadius();
        if (d < minDist && d > 1e-5f)
            return ePos + (toDesired / d) * minDist;
    }

    std::vector<Crystal*> crystals = players[0]->GetCurrentPlanet()->GetCrystals();
    // クリスタルとの当たり判定
    for (auto* crystal : crystals)
    {
        if (!crystal->GetIsActive())
            continue;
        glm::vec3 cPos = crystal->GetPos();
        glm::vec3 toDesired = desiredPos - cPos;
        float d = glm::length(toDesired);
        float minDist = crystal->GetRadius();
        if (d < minDist && d > 1e-5f)
            return cPos + (toDesired / d) * minDist;
    }

    std::vector<NPC*> NPCs = players[0]->GetCurrentPlanet()->GetNPCs();
    // クリスタルとの当たり判定
    for (auto* NPC : NPCs)
    {
        glm::vec3 cPos = NPC->GetPos();
        glm::vec3 toDesired = desiredPos - cPos;
        float d = glm::length(toDesired);
        float minDist = NPC->GetRadius();
        if (d < minDist && d > 1e-5f)
            return cPos + (toDesired / d) * minDist;
    }

    // 壁当たり：球スイープで移動経路に障害があれば移動を打ち切り
    if (mBulletInitialized && mBulletWorld && mBulletWallSphere)
    {
        for (auto player : players) {
            glm::vec3 pos = player->GetPos();
            glm::vec3 upVec = player->GetUpVec();
            glm::vec3 sweepFrom = pos + upVec * 0.4f;
            glm::vec3 sweepTo = desiredPos + upVec * 0.4f;

            btTransform fromBt, toBt;
            fromBt.setIdentity();
            fromBt.setOrigin(btVector3(sweepFrom.x, sweepFrom.y, sweepFrom.z));
            toBt.setIdentity();
            toBt.setOrigin(btVector3(sweepTo.x, sweepTo.y, sweepTo.z));

            btCollisionWorld::ClosestConvexResultCallback sweepCallback(fromBt.getOrigin(),toBt.getOrigin());
            mBulletWorld->convexSweepTest(
                mBulletWallSphere.get(),
                fromBt,
                toBt,
                sweepCallback);

            // 衝突処理
            if (sweepCallback.hasHit())
            {
                // 壁手前で一度止め、残りを壁に沿う方向（スライド）に投影して進める
                float allowFrac = std::max(0.0f, sweepCallback.m_closestHitFraction - 0.02f);
                glm::vec3 posAfterHit = pos + moveDelta * allowFrac;
                glm::vec3 hitNormGlm(
                    sweepCallback.m_hitNormalWorld.x(),
                    sweepCallback.m_hitNormalWorld.y(),
                    sweepCallback.m_hitNormalWorld.z());
                // 阻害された移動を壁面に投影 → 壁沿いのスライドベクトル
                glm::vec3 blocked = moveDelta * (1.0f - allowFrac);
                glm::vec3 slideVec = blocked - hitNormGlm * glm::dot(blocked, hitNormGlm);
                const float slideEps = 1e-4f;
                if (glm::length(slideVec) > slideEps)
                {
                    glm::vec3 slideFrom = posAfterHit + upVec * 0.4f;
                    glm::vec3 slideTo = slideFrom + slideVec;
                    btTransform fromBt2, toBt2;
                    fromBt2.setIdentity();
                    fromBt2.setOrigin(btVector3(slideFrom.x, slideFrom.y, slideFrom.z));
                    toBt2.setIdentity();
                    toBt2.setOrigin(btVector3(slideTo.x, slideTo.y, slideTo.z));
                    btVector3 sFrom(slideFrom.x, slideFrom.y, slideFrom.z);
                    btVector3 sTo(slideTo.x, slideTo.y, slideTo.z);
                    btCollisionWorld::ClosestConvexResultCallback slideCallback(sFrom, sTo);
                    mBulletWorld->convexSweepTest(
                        mBulletWallSphere.get(),
                        fromBt,
                        toBt,
                        sweepCallback);
                    float slideAllow = slideCallback.hasHit()
                                        ? std::max(0.0f, slideCallback.m_closestHitFraction - 0.02f)
                                        : 1.0f;
                    return posAfterHit + slideVec * slideAllow;
                }
                else
                {
                    return posAfterHit;
                }
            } else {
                return desiredPos;
            }
        }
    }
    return desiredPos;
}