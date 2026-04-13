#ifndef PLAYER_H
#define PLAYER_H

#include "Mesh.h"
#include "Planet.h"
#include "Actor.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

class Player : public Actor {
public:
    Player(class Game* game);
    ~Player();

    void ProcessActor() override;
    void UpdateActor(float deltaTime) override;

    // void updatePlayerPhysics(PlayerState& p, float deltaTime, const std::vector<Planet>& planets,
    //     float* transitionTimer = nullptr, bool skipGroundSnap = false);
    
    void getForwardLeft(const glm::vec3& up, float cameraYaw, glm::vec3& outForward, glm::vec3& outLeft);
    float getYawFromDirection(const glm::vec3& up, const glm::vec3& dir);
    glm::mat4 getPlayerView();
    void getPlayerFallbackTriangle(std::vector<float>& outVertices);

    void SetPos(const glm::vec3& pos) { mPos = pos; }
    void SetUpVec(const glm::vec3& upVec) { mUpVec = upVec; }
    void SetKnockBackFrom(const glm::vec3& knockBackFrom) { mKnockBackFrom = knockBackFrom; }
    void SetRestartPos(const glm::vec3& restartPos) { mRestartPos = restartPos; }
    void SetCurrentPlanetNum(int currentPlanetNum) { mCurrentPlanetNum = currentPlanetNum; }
    void SetCameraYaw(float cameraYaw) { mCameraYaw = cameraYaw; }
    void SetCameraPitch(float cameraPitch) { mCameraPitch = cameraPitch; }
    void SetFacingYaw(float facingYaw) { mFacingYaw = facingYaw; }
    void SetVelocity(const glm::vec3& velocity) { mVelocity = velocity; }
    void SetOnGround(bool onGround) { mOnGround = onGround; }
    void SetAttack(float attack) { mAttack = attack; }
    void SetHp(float hp) { mHp = hp; }
    void SetIsDamaged(bool isDamaged) { mIsDamaged = isDamaged; }
    void SetIsDamagePrev(bool isDamagePrev) { mIsDamagePrev = isDamagePrev; }
    void SetDamageTimer(float damageTimer) { mDamageTimer = damageTimer; }
    void SetAttackCooldownRemaining(float attackCooldownRemaining) { mAttackCooldownRemaining = attackCooldownRemaining; }
    void SetAttackMoveLockRemaining(float attackMoveLockRemaining) { mAttackMoveLockRemaining = attackMoveLockRemaining; }
    void SetAttackDodgeLockRemaining(float attackDodgeLockRemaining) { mAttackDodgeLockRemaining = attackDodgeLockRemaining; }
    void SetAttackIndex(int attackIndex) { mAttackIndex = attackIndex; }
    void SetRestartPlanetIndex(int restartPlanetIndex) { mRestartPlanetIndex = restartPlanetIndex; }
    void SetPlayerMeshes(const std::vector<struct LoadedMesh> playerMeshes) { mPlayerMeshes = playerMeshes; }

    const glm::vec3& GetPos() const { return mPos; }
    const glm::vec3& GetUpVec() const { return mUpVec; }
    const glm::vec3& GetKnockBackFrom() const { return mKnockBackFrom; }
    const glm::vec3& GetRestartPos() const { return mRestartPos; }
    int GetCurrentPlanetNum() const { return mCurrentPlanetNum; }
    float GetCameraYaw() const { return mCameraYaw; }
    float GetCameraPitch() const { return mCameraPitch; }
    float GetFacingYaw() const { return mFacingYaw; }
    const glm::vec3& GetVelocity() const { return mVelocity; }
    bool GetOnGround() const { return mOnGround; }
    float GetAttack() const { return mAttack; }
    float GetHp() const { return mHp; }
    bool GetIsDamaged() const { return mIsDamaged; }
    bool GetIsDamagePrev() const { return mIsDamagePrev; }
    float GetDamageTimer() const { return mDamageTimer; }
    float GetAttackCooldownRemaining() const { return mAttackCooldownRemaining; }
    float GetAttackMoveLockRemaining() const { return mAttackMoveLockRemaining; }
    float GetAttackDodgeLockRemaining() const { return mAttackDodgeLockRemaining; }
    int GetAttackIndex() const { return mAttackIndex; }
    int GetRestartPlanetIndex() const { return mRestartPlanetIndex; }

    const std::vector<struct LoadedMesh>& GetPlayerMeshes() const { return mPlayerMeshes; }

private:
    class Planet* mCurrentPlanet;

    glm::vec3 mPos;
    glm::vec3 mUpVec;
    glm::vec3 mKnockBackFrom;
    glm::vec3 mRestartPos;
    glm::vec3 mVelocity;

    int mCurrentPlanetNum;
    int mAttackIndex;
    int mRestartPlanetIndex;

    float mCameraYaw;
    float mCameraPitch;
    float mFacingYaw;
    float mMoveForward;
    float mMoveLeft;
    float mAttackStartHeight;
    float mDodgeTimer;
    float mDodgeCooldown;
    float mMoveSpeed;
    float mCameraStickX;
    float mCameraStickY;
    float mAttack;
    float mHp;
    float mDamageTimer;
    float mAttackCooldownRemaining;
    float mAttackMoveLockRemaining;
    float mAttackDodgeLockRemaining;
    float mAttackHeightLockRemaining;

    bool mOnGround;
    bool mIsDamaged;
    bool mIsDamagePrev;
    bool mDodgePressed;
    bool mDodgePressedPrev;
    bool mJumpPressed;
    bool mAttackPressed; 
    bool mAttackPressedPrev; 
    bool mCounterPressed;
    bool mCounterPressedPrev;

    std::vector<struct LoadedMesh> mPlayerMeshes;
};


#endif
