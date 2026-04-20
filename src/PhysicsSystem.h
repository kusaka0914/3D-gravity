#ifndef PHYSICSSYSTEM_H
#define PHYSICSSYSTEM_H

#include <memory>
#include <vector>

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
private:
    class Game* mGame;

    // Bullet state (owned by this system)
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

    // Local mesh loader for collision cooking (keeps Game API unchanged)
    std::unique_ptr<class Mesh> mMeshLoader;
};
#endif