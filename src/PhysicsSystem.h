#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btTriangleMesh;
class btBvhTriangleMeshShape;
class btRigidBody;
class btPairCachingGhostObject;
class btCapsuleShape;
class btSphereShape;
class btKinematicCharacterController;

class PhysicsSystem {
public:
    PhysicsSystem(class Game* game);
    ~PhysicsSystem();
    void Initialize();
    void Update();

    Game* GetGame() const { return mGame; }
    bool IsBulletInitialized() const { return mBulletInitialized; }
    btDiscreteDynamicsWorld* GetBulletWorld() const { return mBulletWorld; }
    btSphereShape* GetBulletWallSphere() const { return mBulletWallSphere; }

    glm::vec3 CheckCollision(glm::vec3 moveDelta, glm::vec3 desiredPos);
private:
    class Game* mGame;

    bool mBulletInitialized;
    btDefaultCollisionConfiguration* mBulletCollisionConfig;
    btCollisionDispatcher* mBulletDispatcher;
    btBroadphaseInterface* mBulletBroadphase;
    btSequentialImpulseConstraintSolver* mBulletSolver;
    btDiscreteDynamicsWorld* mBulletWorld;

    std::vector<btTriangleMesh*> mBulletPlanetMeshes;
    std::vector<btBvhTriangleMeshShape*> mBulletPlanetShapes;
    std::vector<btRigidBody*> mBulletPlanetBodies;

    btPairCachingGhostObject* mBulletGhost;
    btCapsuleShape* mBulletCapsule;
    btSphereShape* mBulletWallSphere;
    btKinematicCharacterController* mBulletCharController;

    class Mesh* mMeshLoader;
};