#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btDbvtBroadphase;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btGhostPairCallback;
class btTriangleMesh;
class btBvhTriangleMeshShape;
class btRigidBody;
class btPairCachingGhostObject;
class btCapsuleShape;
class btSphereShape;
class btKinematicCharacterController;

class Game;
class Actor;
class MeshLoadSystem;

class PhysicsSystem {
public:
    PhysicsSystem(Game* game);
    ~PhysicsSystem();

    void ClearBulletWorld();
    void Initialize();
    void Update();

    Game* GetGame() const { return mGame; }
    bool IsBulletInitialized() const { return mBulletInitialized; }
    btDiscreteDynamicsWorld* GetBulletWorld() const { return mBulletWorld.get(); }
    btSphereShape* GetBulletWallSphere() const { return mBulletWallSphere.get(); }

    void CreateStageCollisionBodies();
    bool CreatePlayerCharacterController();
    glm::vec3 CheckCollision(glm::vec3 moveDelta, glm::vec3 desiredPos);

private:
    void CreateWorld();
    void CreateBody(Actor* actor);

private:
    Game* mGame;

    bool mBulletInitialized;
    std::unique_ptr<btDefaultCollisionConfiguration> mBulletCollisionConfig;
    std::unique_ptr<btCollisionDispatcher> mBulletDispatcher;
    std::unique_ptr<btDbvtBroadphase> mBulletBroadphase;
    std::unique_ptr<btSequentialImpulseConstraintSolver> mBulletSolver;
    std::unique_ptr<btDiscreteDynamicsWorld> mBulletWorld;

    std::unique_ptr<btGhostPairCallback> mBulletGhostPairCallback;

    std::unique_ptr<btPairCachingGhostObject> mBulletGhost;
    std::unique_ptr<btCapsuleShape> mBulletCapsule;
    std::unique_ptr<btSphereShape> mBulletWallSphere;
    std::unique_ptr<btKinematicCharacterController> mBulletCharController;

    std::vector<std::unique_ptr<btRigidBody>> mBulletPlanetBodies;
    std::vector<std::unique_ptr<btBvhTriangleMeshShape>> mBulletPlanetShapes;
    std::vector<std::unique_ptr<btTriangleMesh>> mBulletPlanetMeshes;

    MeshLoadSystem* mMeshLoadSystem;
};