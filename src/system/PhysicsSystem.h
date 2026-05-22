#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <optional>

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
class Player;

class PhysicsSystem {
public:
    PhysicsSystem(Game* game);
    ~PhysicsSystem();

    void Initialize();

    btDiscreteDynamicsWorld* GetBulletWorld() const { return mBulletWorld.get(); }
    
    glm::vec3 CheckCollision(Player* player, const glm::vec3& moveDelta, const glm::vec3& desiredPos);

private:
    void ClearBulletWorld();
    void CreateWorld();
    void CreateStaticMeshBody(Actor* actor);
    void CreateStageCollisionBodies();
    void CreatePlayerShape();
    std::unique_ptr<btTriangleMesh> CreateTriangleMesh(const glm::vec3& actorScale, const std::vector<float>& pos, const std::vector<unsigned int>& idx);

    std::optional<glm::vec3> CheckConflictActors(Player* player, const glm::vec3& desiredPos);
    std::optional<glm::vec3> CheckConflictActor(Actor* actor, const glm::vec3& desiredPos);
    std::optional<glm::vec3> CheckConflictWall(Player* player, const glm::vec3& moveDelta, const glm::vec3& desiredPos);

private:
    Game* mGame;

    std::unique_ptr<btDefaultCollisionConfiguration> mBulletCollisionConfig;
    std::unique_ptr<btCollisionDispatcher> mBulletDispatcher;
    std::unique_ptr<btDbvtBroadphase> mBulletBroadphase;
    std::unique_ptr<btSequentialImpulseConstraintSolver> mBulletSolver;
    std::unique_ptr<btDiscreteDynamicsWorld> mBulletWorld;

    std::unique_ptr<btSphereShape> mPlayerShape;

    std::vector<std::unique_ptr<btRigidBody>> mBulletRigidBodies;
    std::vector<std::unique_ptr<btBvhTriangleMeshShape>> mBulletTriangleMeshShapes;
    std::vector<std::unique_ptr<btTriangleMesh>> mBulletTriangleMeshes;
};