#include "PhysicsSystem.h"
#include "Game.h"
#include "Mesh.h"
#include "Stage.h"
#include "Planet.h"
#include "Player.h"
#include "Enemy.h"

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>

#include <iostream>
#include <string>

PhysicsSystem::PhysicsSystem(Game* game)
    : mGame(game)
    , mBulletInitialized(false)
    , mBulletCollisionConfig(nullptr)
    , mBulletDispatcher(nullptr)
    , mBulletBroadphase(nullptr)
    , mBulletSolver(nullptr)
    , mBulletWorld(nullptr)
    , mBulletGhost(nullptr)
    , mBulletCapsule(nullptr)
    , mBulletWallSphere(nullptr)
    , mBulletCharController(nullptr)
    , mMeshLoader(nullptr)
{
    Initialize();
}

PhysicsSystem::~PhysicsSystem()
{
    if (mBulletWorld && mBulletCharController)
    {
        mBulletWorld->removeAction(mBulletCharController);
    }
    delete mBulletCharController;
    mBulletCharController = nullptr;

    if (mBulletWorld && mBulletGhost)
    {
        mBulletWorld->removeCollisionObject(mBulletGhost);
    }
    delete mBulletGhost;
    mBulletGhost = nullptr;

    delete mBulletWallSphere;
    mBulletWallSphere = nullptr;
    delete mBulletCapsule;
    mBulletCapsule = nullptr;

    if (mBulletWorld)
    {
        for (btRigidBody* body : mBulletPlanetBodies)
        {
            mBulletWorld->removeRigidBody(body);
            delete body;
        }
    }
    mBulletPlanetBodies.clear();

    for (btBvhTriangleMeshShape* shape : mBulletPlanetShapes)
    {
        delete shape;
    }
    mBulletPlanetShapes.clear();

    for (btTriangleMesh* tri : mBulletPlanetMeshes)
    {
        delete tri;
    }
    mBulletPlanetMeshes.clear();

    delete mBulletWorld;
    mBulletWorld = nullptr;
    delete mBulletSolver;
    mBulletSolver = nullptr;
    delete mBulletBroadphase;
    mBulletBroadphase = nullptr;
    delete mBulletDispatcher;
    mBulletDispatcher = nullptr;
    delete mBulletCollisionConfig;
    mBulletCollisionConfig = nullptr;

    mBulletInitialized = false;
}

void PhysicsSystem::Initialize() { 
    const std::vector<Planet*> planets = mGame->GetCurrentStage()->GetPlanets();
    if (!planets.empty())
    {
        
        mMeshLoader = mGame->GetMesh();

        mBulletCollisionConfig = new btDefaultCollisionConfiguration();
        mBulletDispatcher = new btCollisionDispatcher(mBulletCollisionConfig);
        mBulletBroadphase = new btDbvtBroadphase();
        mBulletSolver = new btSequentialImpulseConstraintSolver();
        mBulletWorld = new btDiscreteDynamicsWorld(mBulletDispatcher, mBulletBroadphase, mBulletSolver, mBulletCollisionConfig);
        mBulletWorld->setGravity(btVector3(0, -9.8f, 0));
        mBulletBroadphase->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());

        std::vector<float> pos;
        std::vector<unsigned int> idx;
        // 惑星メッシュから実際の剛体を作成し、物理ワールドに追加する
        for (auto planet : planets)
        {
            std::string meshPath = "../assets/models/" + planet->GetModelPath();
            if (!mMeshLoader->loadMeshPositionsAndIndices(meshPath.c_str(), pos, idx) || pos.size() < 9 || idx.size() < 3)
                continue;

            const glm::vec3& center = planet->GetCenter();
            float radius = planet->GetRadius();
            btTriangleMesh* triMesh = new btTriangleMesh();
            for (size_t i = 0; i + 2 < idx.size(); i += 3)
            {
                unsigned int i0 = idx[i], i1 = idx[i + 1], i2 = idx[i + 2];
                if (i0 * 3 + 2 >= pos.size() || i1 * 3 + 2 >= pos.size() || i2 * 3 + 2 >= pos.size())
                    continue;
                btVector3 v0(center.x + radius * pos[i0 * 3], center.y + radius * pos[i0 * 3 + 1], center.z + radius * pos[i0 * 3 + 2]);
                btVector3 v1(center.x + radius * pos[i1 * 3], center.y + radius * pos[i1 * 3 + 1], center.z + radius * pos[i1 * 3 + 2]);
                btVector3 v2(center.x + radius * pos[i2 * 3], center.y + radius * pos[i2 * 3 + 1], center.z + radius * pos[i2 * 3 + 2]);
                triMesh->addTriangle(v0, v1, v2);
            }

            mBulletPlanetMeshes.emplace_back(triMesh);
            btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(triMesh, true);
            mBulletPlanetShapes.emplace_back(shape);

            btTransform startTransform;
            startTransform.setIdentity();
            startTransform.setOrigin(btVector3(0, 0, 0));

            btRigidBody::btRigidBodyConstructionInfo rbInfo(0, nullptr, shape);
            btRigidBody* body = new btRigidBody(rbInfo);
            body->setWorldTransform(startTransform);
            body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
            mBulletWorld->addRigidBody(body, (short)btBroadphaseProxy::DefaultFilter, (short)-1);
            mBulletPlanetBodies.emplace_back(body);
        }
        mBulletInitialized = !mBulletPlanetBodies.empty();

        const float capRadius = 0.35f;
        const float capHeight = 0.8f;
        mBulletCapsule = new btCapsuleShape(capRadius, capHeight);
        mBulletWallSphere = new btSphereShape(0.35f);
        mBulletGhost = new btPairCachingGhostObject();
        mBulletGhost->setCollisionShape(mBulletCapsule);
        mBulletGhost->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
        btTransform ghostTrans;
        ghostTrans.setIdentity();
        const auto& players = mGame->GetPlayers();
        if (players.empty())
        {
            return;
        }
        glm::vec3 spawnUp = glm::normalize(players[0]->GetPos() - planets[0]->GetCenter());
        float capHalf = capHeight * 0.5f;
        glm::vec3 ghostOrigin = players[0]->GetPos() + spawnUp * (capHalf + 0.15f);
        ghostTrans.setOrigin(btVector3(ghostOrigin.x, ghostOrigin.y, ghostOrigin.z));
        mBulletGhost->setWorldTransform(ghostTrans);
        mBulletWorld->addCollisionObject(mBulletGhost, (short)btBroadphaseProxy::CharacterFilter, (short)-1);
        mBulletCharController = new btKinematicCharacterController(mBulletGhost, mBulletCapsule, 0.35f);
        mBulletCharController->setGravity(btVector3(0, -9.8f, 0));
        mBulletCharController->setJumpSpeed(5.0f);
        mBulletCharController->setFallSpeed(55.0f);
        mBulletWorld->addAction(mBulletCharController);
    }
    else
    {
        if (!mBulletInitialized)
        {
            std::cerr << "Bullet: planet mesh load failed, using sphere collision." << std::endl;
        }
        mBulletInitialized = false;
    }
}

void PhysicsSystem::Update()
{
    if (!mBulletWorld)
        return;

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
            mBulletWorld->convexSweepTest(mBulletWallSphere, fromBt, toBt, sweepCallback);

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
                    mBulletWorld->convexSweepTest(mBulletWallSphere, fromBt2, toBt2, slideCallback);
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
}