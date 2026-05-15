#pragma once

#include "actor/CharacterActor.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

class Game;
class NPC;
class Boat;
class Enemy;

class Player : public CharacterActor {
public:
    enum class ActionState {
        Idle,
        Dodging,
        Attacking,
        Charging,
        StrongAttacking,
        KnockedBack
    };

    enum class AttackKind {
        Normal,
        Wide,
        Strong
    };

    struct RaySegment {
        glm::vec3 from;
        glm::vec3 to;
    };

    Player(Game* game);
    ~Player();

    void Initialize() override;
    void ProcessActor() override;
    void UpdateActor(float deltaTime) override;

    void ApplyDamage(float damage, glm::vec3 knockBackFrom);
    void OnBoatArrived(Boat* boat);

    void SetCanDodge(bool canDodge) { mCanDodge = canDodge; }

    void SetCurrentPlanetNum(int currentPlanetNum) { mCurrentPlanetNum = currentPlanetNum; }
    void SetPlayerNum(int playerNum) { mPlayerNum = playerNum; }

    void SetCameraPitch(float cameraPitch) { mCameraPitch = cameraPitch; }
    void SetAttack(float attack) { mAttack = attack; }
    void SetMoveSpeed(float moveSpeed) { mMoveSpeed = moveSpeed; }
    void SetAttackSpeed(float attackSpeed) { mAttackSpeed = attackSpeed; }
    void SetChargeMoveSpeed(float chargeMoveSpeed) { mChargeMoveSpeed = chargeMoveSpeed; }
    void SetHp(float hp) { mHp = hp; }  
    void SetMaxHp(float maxHp) { mMaxHp = maxHp; }  
    void SetDefaultDamageTimer(float defaultDamageTimer) { mDefaultDamageTimer = defaultDamageTimer; }
    void SetAttackCooldown(float attackCooldown) { mAttackCooldown = attackCooldown; }
    void SetLastAttackCooldown(float lastAttackCooldown) { mLastAttackCooldown = lastAttackCooldown; }
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

    void SetVelocity(const glm::vec3& velocity) { mVelocity = velocity; }

    void SetTalkingNPC(NPC* talkingNPC) { mTalkingNPC = talkingNPC; }

    int GetCurrentPlanetNum() const { return mCurrentPlanetNum; }

    float GetAttack() const { return mAttack; }
    float GetHp() const { return mHp; }
    float GetAttackMotionTimer() const { return mAttackMotionTimer; }
    float GetStrongAttackTimer() const { return mStrongAttackTimer; }
    float GetInvincibleTimer() const { return mDefaultInvincibleTimer; }
    float GetSpecialAttackCooldownRemaining() const { return mSpecialAttackCooldownRemaining; }
    float GetAttackRange() const { return mAttackRange; }
    float GetRayCastTimer() const { return mRayCastTimer; }

    ActionState GetActionState() const { return mActionState; }

    const glm::vec3& GetForwardVec() const { return mForwardVec; }
    const glm::vec3& GetFacingForwardVec() const { return mFacingForwardVec; }
    const std::vector<RaySegment>& GetRayCasts() const { return mRayCasts; }

    NPC* GetTalkingNPC() const { return mTalkingNPC; }

private:
    void ProcessGameController();
    void ProcessKeyboard();

    void UpdateAlive(float deltaTime);
    void Die();

    void UpdateWorldVec();
    void UpdateIdle(float deltaTime);
    void UpdateDodging(float deltaTime);
    void UpdateAttacking(float deltaTime);
    void UpdateCharging(float deltaTime);
    void UpdateStrongAttacking(float deltaTime);
    void UpdateKnockedBack(float deltaTime);
    void UpdateTimer(float deltaTime);

    void UpdateWalk(float deltaTime);
    void UpdateBoatRide();

    void StartIdle();    
    void StartDodging();
    void StartAttacking(float deltaTime);
    void StartCharging(float deltaTime);
    void StartStrongAttacking();
    void StartRidingBoat(Boat* boat);
    void StartJumping(float deltaTime);
    
    void FinishCharging();

    void ApplyGravity(float deltaTime);
    void ChangeFaceDir();
    void Respawn();
    void MoveDuringDodging(float deltaTime);
    void MoveDuringAttacking(float deltaTime);
    void MoveDuringCharging(float deltaTime);
    void MoveDuringStrongAttacking(float deltaTime);
    void Attack(float deltaTime);
    void WideAttack(float deltaTime);
    void StrongAttack(float deltaTime);
    void MoveDuringKnockBack(float deltaTime);
    void SpecialAttack(float deltaTime);
    void FollowMovingBoat(Boat* boat);
    void FixPlanetSurface();

    bool IsAlive() const { return mHp >= 0.0f; };
    bool IsTouchingBoat(Boat* boat);
    bool IsEnemyHitByAttack(float dist, float dot, float effectiveRange);
    bool CanWalk() const { return mAttackMoveLockRemaining <= 0.0f; }
    std::vector<Enemy*> FindHitEnemies();


private:
    ActionState mActionState;
    AttackKind mAttackKind;

    bool mDodgePressed;
    bool mJumpPressed;
    bool mAttackPressed; 
    bool mWideAttackPressed;
    bool mSpecialAttackPressed;
    bool mCanDodge;

    int mCurrentPlanetNum;
    int mAttackIndex;
    int mRestartPlanetIndex;
    int mPlayerNum;

    float mCameraYaw;
    float mCameraPitch;
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
    glm::vec3 mKnockBackFrom;
    glm::vec3 mRestartPos;
    glm::vec3 mDodgeDir;
    std::vector<RaySegment> mRayCasts;

    NPC* mTalkingNPC;
};