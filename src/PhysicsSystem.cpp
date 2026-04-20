#include "PhysicsSystem.h"
#include "Game.h"
#include "Mesh.h"
#include "Stage.h"
#include "Planet.h"
#include "Player.h"

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>

#include <glm/glm.hpp>
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
    // Note: we only ever add static rigid bodies and one ghost/controller here.
    // Tear down in reverse order of creation.
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
    // Bullet Physics：惑星メッシュの当たり判定（惑星ごとに modelPath のメッシュを使用）
    if (!mGame || !mGame->GetCurrentStage())
    {
        mBulletInitialized = false;
        return;
    }

    const std::vector<Planet*> planets = mGame->GetCurrentStage()->GetPlanets();
    if (!planets.empty())
    {
        // (Re)create mesh loader if needed
        if (!mMeshLoader)
            mMeshLoader = std::make_unique<Mesh>();

        mBulletCollisionConfig = new btDefaultCollisionConfiguration();
        mBulletDispatcher = new btCollisionDispatcher(mBulletCollisionConfig);
        mBulletBroadphase = new btDbvtBroadphase();
        mBulletSolver = new btSequentialImpulseConstraintSolver();
        mBulletWorld = new btDiscreteDynamicsWorld(mBulletDispatcher, mBulletBroadphase, mBulletSolver, mBulletCollisionConfig);
        mBulletWorld->setGravity(btVector3(0, -9.8f, 0));
        mBulletBroadphase->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());

        std::vector<float> pos;
        std::vector<unsigned int> idx;
        for (size_t p = 0; p < planets.size(); p++)
        {
            pos.clear();
            idx.clear();
            std::string meshPath = "../assets/models/" + planets[p]->GetModelPath();
            if (!mMeshLoader->loadMeshPositionsAndIndices(meshPath.c_str(), pos, idx) || pos.size() < 9 || idx.size() < 3)
                continue;
            const glm::vec3 &center = planets[p]->GetCenter();
            float radius = planets[p]->GetRadius();
            btTriangleMesh *triMesh = new btTriangleMesh();
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
            btBvhTriangleMeshShape *shape = new btBvhTriangleMeshShape(triMesh, true);
            mBulletPlanetShapes.emplace_back(shape);
            btTransform startTransform;
            startTransform.setIdentity();
            startTransform.setOrigin(btVector3(0, 0, 0));
            btRigidBody::btRigidBodyConstructionInfo rbInfo(0, nullptr, shape);
            btRigidBody *body = new btRigidBody(rbInfo);
            body->setWorldTransform(startTransform);
            body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
            mBulletWorld->addRigidBody(body, (short)btBroadphaseProxy::DefaultFilter, (short)-1);
            mBulletPlanetBodies.emplace_back(body);
        }
        mBulletInitialized = !mBulletPlanetBodies.empty();

        const float capRadius = 0.35f;
        const float capHeight = 0.8f;
        mBulletCapsule = new btCapsuleShape(capRadius, capHeight);
        mBulletWallSphere = new btSphereShape(0.35f); // 壁スイープ用（キャラ半径程度）
        mBulletGhost = new btPairCachingGhostObject();
        mBulletGhost->setCollisionShape(mBulletCapsule);
        mBulletGhost->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
        btTransform ghostTrans;
        ghostTrans.setIdentity();
        const auto& players = mGame->GetPlayers();
        if (players.empty())
        {
            // Can't place a character controller without a player spawn; keep world for static collision only.
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

    // TODO: thread dt from Game; using fixed step for now.
    constexpr btScalar fixedTimeStep = btScalar(1.0f / 60.0f);
    constexpr int maxSubSteps = 1;
    mBulletWorld->stepSimulation(fixedTimeStep, maxSubSteps);
}