#pragma once

#include "CharacterActor.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class Game;
class Player;

class Enemy : public CharacterActor {
public:
    enum class LifeState {
        Alive,
        Dying,
        Dead
    };

    enum class ActionState {
        Idle,
        Tracking,
        PreparingAttack,
        Attacking,
        KnockedBack,
    };

    Enemy(Game* game);
    void UpdateActor(float deltaTime) override;

    void ApplyDamage(float damage);
    void ApplyBreak(float deltaTime);

    void SetIsBoss(bool isBoss) { mIsBoss = isBoss; }
    void SetIsStrongAttacked(bool isStrongAttacked) { mIsStrongAttacked = isStrongAttacked; }

    void SetBreakCount(int breakCount) { mBreakCount = breakCount; }
    void SetBreakCountMax(int breakCountMax) { mBreakCountMax = breakCountMax; }

    void SetHp(float hp) { mHp = hp; }
    void SetMaxHp(float maxHp) { mMaxHp = maxHp; }
    void SetDefaultLaunchedTimer(float defaultLaunchedTimer) { mDefaultLaunchedTimer = defaultLaunchedTimer; }
    void SetMoveSpeed(float moveSpeed) { mMoveSpeed = moveSpeed; }
    void SetAttack(float attack) { mAttack = attack; }
    void SetDefaultAttackMotionTimer(float defaultAttackMotionTimer) { mDefaultAttackMotionTimer = defaultAttackMotionTimer; }
    void SetDefaultStandByAttackTimer(float defaultStandByAttackTimer) { mDefaultStandByAttackTimer = defaultStandByAttackTimer; }
    void SetDetectionRange(float detectionRange) { mDetectionRange = detectionRange; }
    void SetKnockBackSpeed(float knockBackSpeed) { mKnockBackSpeed = knockBackSpeed; }
    void SetAttackSpeed(float attackSpeed) { mAttackSpeed = attackSpeed; }

    bool GetIsDead() const { return mLifeState == LifeState::Dead; }

    int GetBreakCount() const { return mBreakCount; }
    
    float GetHp() const { return mHp; }
    float GetMaxHp() const { return mMaxHp; }

private:
    void UpdateAlive(float deltaTime);
    void UpdateDying(float deltaTime);

    void UpdateBehavior(float deltaTime, Player* player);
    void UpdateIdle(Player* player);
    void UpdateTracking(float deltaTime, Player* player);
    void UpdatePreparingAttack(float deltaTime);
    void UpdateAttacking(float deltaTime, Player* player);
    void UpdateKnockedBack(float deltaTime, Player* player);

    void StartIdle();
    void StartTracking();
    void StartPreparingAttack();
    void StartAttacking();
    void StartKnockedBack(float knockBackTimer);
    void StartDying();
    
    void FinishLaunched();
    void FinishDying();

    bool IsPlayerInRange(float range, Player* player);
    bool IsJustBeforeAttack();

    void MoveToPlayer(float deltaTime, Player* player);
    void MoveDuringAttacking(float deltaTime, Player* player);
    void MoveDuringKnockBack(float deltaTime, Player* player);
    void LaunchIntoAir(float deltaTime);
    void ApplyCounter(Player* player);
    void ApplyGravity(float deltaTime);

private:
    LifeState mLifeState;
    ActionState mActionState;

    bool mIsCountered;
    bool mIsBoss;
    bool mIsHit;
    bool mIsStrongAttacked;
    bool mIsJustBeforeAttack;

    int mBreakCount;
    int mBreakCountMax;

    float mAttack;
    float mHp;
    float mMaxHp;
    float mDetectionRange;
    float mMoveSpeed;
    float mKnockBackSpeed;
    float mAttackSpeed;

    float mStandByAttackTimer;
    float mDefaultStandByAttackTimer;
    float mLaunchedTimer;
    float mDefaultLaunchedTimer;
    float mAttackMotionTimer;
    float mDefaultAttackMotionTimer;
    float mDyingTimer;
    float mKnockBackTimer;
};