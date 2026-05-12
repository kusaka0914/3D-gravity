#pragma once

#include "system/MeshLoadSystem.h"
#include "actor/Planet.h"
#include "actor/CharacterActor.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

class Player : public CharacterActor {
public:
    Player(class Game* game);
    ~Player();

    struct RaySegment {
        glm::vec3 from;
        glm::vec3 to;
    };

    void ProcessActor() override;
    void UpdateActor(float deltaTime) override;
    
    glm::mat4 getPlayerView(float cameraDistance, bool isFixed = false);

    void SetIsDamaged(bool isDamaged) { mIsDamaged = isDamaged; }
    void SetIsDamagePrev(bool isDamagePrev) { mIsDamagePrev = isDamagePrev; }
    void SetCanMove(bool canMove) { mCanMove = canMove; }
    void SetCanDodge(bool canDodge) { mCanDodge = canDodge; }

    void SetCurrentPlanetNum(int currentPlanetNum) { mCurrentPlanetNum = currentPlanetNum; }
    void SetPlayerNum(int playerNum) { mPlayerNum = playerNum; }
    void SetAttackIndex(int attackIndex) { mAttackIndex = attackIndex; }
    void SetRestartPlanetIndex(int restartPlanetIndex) { mRestartPlanetIndex = restartPlanetIndex; }

    void SetCameraYaw(float cameraYaw) { mCameraYaw = cameraYaw; }
    void SetCameraPitch(float cameraPitch) { mCameraPitch = cameraPitch; }
    void SetFacingYaw(float facingYaw) { mFacingYaw = facingYaw; }
    void SetAttack(float attack) { mAttack = attack; }
    void SetMoveSpeed(float moveSpeed) { mMoveSpeed = moveSpeed; }
    void SetAttackSpeed(float attackSpeed) { mAttackSpeed = attackSpeed; }
    void SetChargeMoveSpeed(float chargeMoveSpeed) { mChargeMoveSpeed = chargeMoveSpeed; }
    void SetHp(float hp) { mHp = hp; }  
    void SetMaxHp(float maxHp) { mMaxHp = maxHp; }  
    void SetDamageTimer(float damageTimer) { mDamageTimer = damageTimer; }
    void SetDefaultDamageTimer(float defaultDamageTimer) { mDefaultDamageTimer = defaultDamageTimer; }
    void SetAttackCooldownRemaining(float attackCooldownRemaining) { mAttackCooldownRemaining = attackCooldownRemaining; }
    void SetAttackCooldown(float attackCooldown) { mAttackCooldown = attackCooldown; }
    void SetLastAttackCooldown(float lastAttackCooldown) { mLastAttackCooldown = lastAttackCooldown; }
    void SetAttackMoveLockRemaining(float attackMoveLockRemaining) { mAttackMoveLockRemaining = attackMoveLockRemaining; }
    void SetAttackDodgeLockRemaining(float attackDodgeLockRemaining) { mAttackDodgeLockRemaining = attackDodgeLockRemaining; }
    void SetDefaultAttackPressTimer(float defaultAttackPressTimer) { mDefaultAttackPressTimer = defaultAttackPressTimer; }
    void SetSpecialAttackCooldown(float specialAttackCooldown) { mSpecialAttackCooldown = specialAttackCooldown; }
    void SetDodgeDuration(float dodgeDuration) { mDodgeDuration = dodgeDuration; }
    void SetDefaultInvincibleTimer(float defaultInvincibleTimer) { mDefaultInvincibleTimer = defaultInvincibleTimer; }
    void SetDodgeCooldownTime(float dodgeCooldownTime) { mDodgeCooldownTime = dodgeCooldownTime; }
    void SetDodgeDistance(float dodgeDistance) { mDodgeDistance = dodgeDistance; }
    void SetNormalAttackRange(float normalAttackRange) { mNormalAttackRange = normalAttackRange; }
    void SetNormalAttackAngle(float normalAttackAngle) { mNormalAttackAngle = normalAttackAngle; }
    void SetNormalAttack(float normalAttack) { mNormalAttack = normalAttack; }
    void SetWideAttackRange(float wideAttackRange) { mWideAttackRange = wideAttackRange; }
    void SetWideAttackAngle(float wideAttackAngle) { mWideAttackAngle = wideAttackAngle; }
    void SetWideAttack(float wideAttack) { mWideAttack = wideAttack; }
    void SetStrongAttackRange(float strongAttackRange) { mStrongAttackRange = strongAttackRange; }
    void SetStrongAttack(float strongAttack) { mStrongAttack = strongAttack; }
    void SetStrongAttackSpeed(float strongAttackSpeed) { mStrongAttackSpeed = strongAttackSpeed; }
    void SetDefaultStrongAttackTimer(float defaultStrongAttackTimer) { mDefaultStrongAttackTimer = defaultStrongAttackTimer; }
    void SetDefaultAttackMotionTimer(float defaultAttackMotionTimer) { mDefaultAttackMotionTimer = defaultAttackMotionTimer; }
    void SetRayCastTimer(float rayCastTimer) { mRayCastTimer = rayCastTimer; }

    void SetKnockBackFrom(const glm::vec3& knockBackFrom) { mKnockBackFrom = knockBackFrom; }
    void SetRestartPos(const glm::vec3& restartPos) { mRestartPos = restartPos; }
    void SetVelocity(const glm::vec3& velocity) { mVelocity = velocity; }
    void SetCameraTargetPos(const glm::vec3& cameraTargetPos) { mCameraTargetPos = cameraTargetPos; }
    void SetCameraPos(const glm::vec3& cameraPos) { mCameraPos = cameraPos; }
    void SetCameraUpVec(const glm::vec3& cameraUpVec) { mCameraUpVec = cameraUpVec; }

    void SetTalkingNPC(class NPC* talkingNPC) { mTalkingNPC = talkingNPC; }

    bool GetIsDamaged() const { return mIsDamaged; }
    bool GetIsDamagePrev() const { return mIsDamagePrev; }

    int GetCurrentPlanetNum() const { return mCurrentPlanetNum; }
    int GetAttackIndex() const { return mAttackIndex; }
    int GetRestartPlanetIndex() const { return mRestartPlanetIndex; }

    float GetCameraYaw() const { return mCameraYaw; }
    float GetCameraPitch() const { return mCameraPitch; }
    float GetFacingYaw() const { return mFacingYaw; }
    float GetAttack() const { return mAttack; }
    float GetMoveSpeed() const { return mMoveSpeed; }
    float GetAttackSpeed() const { return mAttackSpeed; }
    float GetChargeMoveSpeed() const { return mChargeMoveSpeed; }
    float GetHp() const { return mHp; }
    float GetMaxHp() const { return mMaxHp; }
    float GetDamageTimer() const { return mDamageTimer; }
    float GetDefaultDamageTimer() const { return mDefaultDamageTimer; }
    float GetAttackCooldownRemaining() const { return mAttackCooldownRemaining; }
    float GetAttackCooldown() const { return mAttackCooldown; }
    float GetLastAttackCooldown() const { return mLastAttackCooldown; }
    float GetAttackMoveLockRemaining() const { return mAttackMoveLockRemaining; }
    float GetAttackDodgeLockRemaining() const { return mAttackDodgeLockRemaining; }
    float GetAttackMotionTimer() const { return mAttackMotionTimer; }
    float GetDefaultAttackMotionTimer() const { return mDefaultAttackMotionTimer; }
    float GetStrongAttackTimer() const { return mStrongAttackTimer; }
    float GetInvincibleTimer() const { return mDefaultInvincibleTimer; }
    float GetDefaultInvincibleTimer() const { return mInvincibleTimer; }
    float GetSpecialAttackCooldownRemaining() const { return mSpecialAttackCooldownRemaining; }
    float GetSpecialAttackCooldown() const { return mSpecialAttackCooldown; }
    float GetDefaultAttackPressTimer() const { return mDefaultAttackPressTimer; }
    float GetAttackRange() const { return mAttackRange; }
    float GetAttackAngle() const { return mAttackAngle; }
    float GetDodgeDuration() const { return mDodgeDuration; }
    float GetDodgeCooldownTime() const { return mDodgeCooldownTime; }
    float GetDodgeDistance() const { return mDodgeDistance; }
    float GetNormalAttackRange() const { return mNormalAttackRange; }
    float GetNormalAttackAngle() const { return mNormalAttackAngle; }
    float GetNormalAttack() const { return mNormalAttack; }
    float GetWideAttackRange() const { return mWideAttackRange; }
    float GetWideAttackAngle() const { return mWideAttackAngle; }
    float GetWideAttack() const { return mWideAttack; }
    float GetStrongAttackRange() const { return mStrongAttackRange; }
    float GetStrongAttack() const { return mStrongAttack; }
    float GetStrongAttackSpeed() const { return mStrongAttackSpeed; }
    float GetDefaultStrongAttackTimer() const { return mDefaultStrongAttackTimer; }
    float GetRayCastTimer() const { return mRayCastTimer; }

    const glm::vec3& GetForwardVec() const { return mForwardVec; }
    const glm::vec3& GetLeftVec() const { return mLeftVec; }
    const glm::vec3& GetFacingForwardVec() const { return mFacingForwardVec; }
    const glm::vec3& GetFacingLeftVec() const { return mFacingLeftVec; }
    const glm::vec3& GetKnockBackFrom() const { return mKnockBackFrom; }
    const glm::vec3& GetRestartPos() const { return mRestartPos; }
    const glm::vec3& GetCameraPos() const { return mCameraPos; }
    const glm::vec3& GetCameraTargetPos() const { return mCameraTargetPos; }
    const std::vector<RaySegment>& GetRayCasts() const { return mRayCasts; }

    NPC* GetTalkingNPC() const { return mTalkingNPC; }

private:
    void ProcessGameController();
    void ProcessKeyboard();

    void UpdateCamera(float deltaTime);
    void UpdateWorldVec();
    void UpdateWalk(float deltaTime);
    void UpdateDodge(float deltaTime);
    void StartDodge();
    void Dodge(float deltaTime);
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
    void UpdateCameraSmoothing(float deltaTime);

private:
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
    bool mCanMove;
    bool mCanDodge;
    bool mIsStrongAttack;

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
    float mDodgeDuration;
    float mDodgeCooldown;
    float mDodgeCooldownTime;
    float mDodgeDistance;
    float mDodgeStartHeight;
    float mMoveSpeed;
    float mChargeMoveSpeed;
    float mCameraStickX;
    float mCameraStickY;
    float mAttack;
    float mAttackSpeed;
    float mHp;
    float mMaxHp;
    float mDamageTimer;
    float mDefaultDamageTimer;
    float mAttackCooldownRemaining;
    float mAttackCooldown;
    float mLastAttackCooldown;
    float mAttackMoveLockRemaining;
    float mAttackDodgeLockRemaining;
    float mAttackMotionTimer;
    float mDefaultAttackMotionTimer;
    float mSpecialAttackCooldownRemaining;
    float mSpecialAttackCooldown;
    float mAttackPressTimer;
    float mDefaultAttackPressTimer;
    float mStrongAttackTimer;
    float mDefaultStrongAttackTimer;
    float mComboTimer;
    float mInvincibleTimer;
    float mDefaultInvincibleTimer;
    float mAttackRange;
    float mAttackAngle;
    float mNormalAttackRange;
    float mNormalAttackAngle;
    float mNormalAttack;
    float mWideAttackRange;
    float mWideAttackAngle;
    float mWideAttack;
    float mStrongAttackRange;
    float mStrongAttack;
    float mStrongAttackSpeed;
    float mRayCastTimer;

    glm::vec3 mForwardVec;
    glm::vec3 mLeftVec;
    glm::vec3 mFacingForwardVec;
    glm::vec3 mFacingLeftVec;
    glm::vec3 mKnockBackFrom;
    glm::vec3 mRestartPos;
    glm::vec3 mDodgeDir;
    glm::vec3 mCameraPos;
    std::vector<RaySegment> mRayCasts;

    glm::vec3 mCameraUpVec;
    glm::vec3 mCameraTargetPos;

    class NPC* mTalkingNPC;
};