#pragma once

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
    
    float getYawFromDirection(const glm::vec3& up, const glm::vec3& dir);
    glm::mat4 getPlayerView(float cameraDistance, bool isFixed = false);

    void SetCurrentPlanet(Planet* currentPlanet) { mCurrentPlanet = currentPlanet; }
    void SetPos(const glm::vec3& pos) { mPos = pos; }
    void SetUpVec(const glm::vec3& upVec) { mUpVec = upVec; }
    void SetKnockBackFrom(const glm::vec3& knockBackFrom) { mKnockBackFrom = knockBackFrom; }
    void SetRestartPos(const glm::vec3& restartPos) { mRestartPos = restartPos; }
    void SetCurrentPlanetNum(int currentPlanetNum) { mCurrentPlanetNum = currentPlanetNum; }
    void SetPlayerNum(int playerNum) { mPlayerNum = playerNum; }
    void SetCameraYaw(float cameraYaw) { mCameraYaw = cameraYaw; }
    void SetCameraPitch(float cameraPitch) { mCameraPitch = cameraPitch; }
    void SetFacingYaw(float facingYaw) { mFacingYaw = facingYaw; }
    void SetVelocity(const glm::vec3& velocity) { mVelocity = velocity; }
    void SetOnGround(bool onGround) { mOnGround = onGround; }
    void SetAttack(float attack) { mAttack = attack; }
    void SetHp(float hp) { mHp = hp; }
    void SetIsDamaged(bool isDamaged) { mIsDamaged = isDamaged; }
    void SetIsDamagePrev(bool isDamagePrev) { mIsDamagePrev = isDamagePrev; }
    void SetIsActive(bool isActive) override { mIsActive = isActive; }
    void SetCanMove(bool canMove) { mCanMove = canMove; }
    void SetDamageTimer(float damageTimer) { mDamageTimer = damageTimer; }
    void SetAttackCooldownRemaining(float attackCooldownRemaining) { mAttackCooldownRemaining = attackCooldownRemaining; }
    void SetAttackMoveLockRemaining(float attackMoveLockRemaining) { mAttackMoveLockRemaining = attackMoveLockRemaining; }
    void SetAttackDodgeLockRemaining(float attackDodgeLockRemaining) { mAttackDodgeLockRemaining = attackDodgeLockRemaining; }
    void SetAttackIndex(int attackIndex) { mAttackIndex = attackIndex; }
    void SetRestartPlanetIndex(int restartPlanetIndex) { mRestartPlanetIndex = restartPlanetIndex; }
    void SetModelPath(std::string modelPath) { mModelPath = modelPath; }
    void SetMeshes(const std::vector<struct LoadedMesh> meshes) { mMeshes = meshes; }
    void SetTalkingNPC(class NPC* talkingNPC) { mTalkingNPC = talkingNPC; }

    Planet* GetCurrentPlanet() const { return mCurrentPlanet; }
    const glm::vec3& GetPos() const override { return mPos; }
    const glm::vec3& GetUpVec() const { return mUpVec; }
    const glm::vec3& GetForwardVec() const { return mForwardVec; }
    const glm::vec3& GetLeftVec() const { return mLeftVec; }
    const glm::vec3& GetFacingForwardVec() const { return mFacingForwardVec; }
    const glm::vec3& GetFacingLeftVec() const { return mFacingLeftVec; }
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
    bool GetIsActive() const { return mIsActive; }
    float GetDamageTimer() const { return mDamageTimer; }
    float GetAttackCooldownRemaining() const { return mAttackCooldownRemaining; }
    float GetAttackMoveLockRemaining() const { return mAttackMoveLockRemaining; }
    float GetAttackDodgeLockRemaining() const { return mAttackDodgeLockRemaining; }
    float GetAttackMotionTimer() const { return mAttackMotionTimer; }
    float GetStrongAttackTimer() const { return mStrongAttackTimer; }
    float GetInvincibleTimer() const { return mInvincibleTimer; }
    float GetSpecialAttackCooldownRemaining() const { return mSpecialAttackCooldownRemaining; }
    int GetAttackIndex() const { return mAttackIndex; }
    int GetRestartPlanetIndex() const { return mRestartPlanetIndex; }
    std::string GetModelPath() const { return mModelPath; }
    NPC* GetTalkingNPC() const { return mTalkingNPC; }

    const std::vector<struct LoadedMesh>& GetMeshes() const { return mMeshes; }

private:
    void ProcessGameController();
    void ProcessKeyboard();

    void UpdateCamera(float deltaTime);
    void UpdateWorldVec();
    void UpdateWalk(float deltaTime);
    void UpdateDodge(float deltaTime);
    void StartDodge(float dodgeDuration, float dodgeCooldownTime);
    void Dodge(float deltaTime, float dodgeDuration);
    void DetermineLanding();
    void ApplyGravity(float deltaTime);
    void ChangeFaceDir();
    void Attack(float deltaTime);
    void SpecialAttack();
    void ChargeAttack(float deltaTime);
    void TakeDamage();
    void Die();
    void RideBoat();
    void UpdateTimer(float deltaTime);
    void UpdatePrev();

private:
    bool mOnGround;
    bool mIsDamaged;
    bool mIsDamagePrev;
    bool mDodgePressed;
    bool mDodgePressedPrev;
    bool mJumpPressed;
    bool mAttackPressed; 
    bool mAttackPressedPrev; 
    bool mWideAttackPressed;
    bool mWideAttackPressedPrev;
    bool mSpecialAttackPressed;
    bool mSpecialAttackPressedPrev;
    bool mIsActive;
    bool mCanMove;

    int mCurrentPlanetNum;
    int mAttackIndex;
    int mAirAttackIndex;
    int mRestartPlanetIndex;
    int mPlayerNum;

    float mCameraYaw;
    float mCameraPitch;
    float mFacingYaw;
    float mMoveForward;
    float mMoveLeft;
    float mAttackStartHeight;
    float mDodgeTimer;
    float mDodgeCooldown;
    float mDodgeStartHeight;
    float mMoveSpeed;
    float mCameraStickX;
    float mCameraStickY;
    float mAttack;
    float mHp;
    float mDamageTimer;
    float mAttackCooldownRemaining;
    float mAttackMoveLockRemaining;
    float mAttackDodgeLockRemaining;
    float mAttackMotionTimer;
    float mSpecialAttackCooldownRemaining;
    float mAttackPressTimer;
    float mStrongAttackTimer;
    float mComboTimer;
    float mInvincibleTimer;

    glm::vec3 mPos;
    glm::vec3 mUpVec;
    glm::vec3 mForwardVec;
    glm::vec3 mLeftVec;
    glm::vec3 mFacingForwardVec;
    glm::vec3 mFacingLeftVec;
    glm::vec3 mKnockBackFrom;
    glm::vec3 mRestartPos;
    glm::vec3 mVelocity;
    glm::vec3 mDodgeDir;

    std::string mModelPath;

    class Planet* mCurrentPlanet;
    class NPC* mTalkingNPC;

    std::vector<struct LoadedMesh> mMeshes;
};