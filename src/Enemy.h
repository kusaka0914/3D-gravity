#pragma once

#include "Actor.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class Enemy : public Actor {
public:
    Enemy(class Game* game);
    void UpdateActor(float deltaTime) override;

    virtual bool IsBoss() const { return false; }

    
    void SetIsAttack(bool isAttack) { mIsAttack = isAttack; }
    void SetIsAlive(bool isAlive) { mIsAlive = isAlive; }
    void SetIsDamaged(bool isDamaged) { mIsDamaged = isDamaged; }
    void SetIsCountered(bool isCountered) { mIsCountered = isCountered; }
    void SetIsBoss(bool isBoss) { mIsBoss = isBoss; }
    void SetIsBroken(bool isLaunched) { mIsBroken = isLaunched; }
    void SetIsStrongAttacked(bool isStrongAttacked) { mIsStrongAttacked = isStrongAttacked; }

    void SetCurrentPlanetNum(int currentPlanetNum) { mCurrentPlanetNum = currentPlanetNum; }
    void SetBreakCount(int breakCount) { mBreakCount = breakCount; }
    void SetBreakCountMax(int breakCountMax) { mBreakCountMax = breakCountMax; }

    void SetHp(float hp) { mHp = hp; }
    void SetMaxHp(float maxHp) { mMaxHp = maxHp; }
    void SetDeathTimer(float damageTimer) { mDeathTimer = damageTimer; }
    void SetLaunchedTimer(float launchedTimer) { mLaunchedTimer = launchedTimer; }
    void SetDefaultLaunchedTimer(float defaultLaunchedTimer) { mDefaultLaunchedTimer = defaultLaunchedTimer; }
    void SetRadius(float radius) { mRadius = radius; }
    void SetSpeed(float speed) { mSpeed = speed; }
    void SetAttack(float attack) { mAttack = attack; }
    void SetDefaultAttackMotionTimer(float defaultAttackMotionTimer) { mDefaultAttackMotionTimer = defaultAttackMotionTimer; }
    void SetStandByAttackTimer(float standByAttackTimer) { mStandByAttackTimer = standByAttackTimer; }
    void SetDefaultStandByAttackTimer(float defaultStandByAttackTimer) { mDefaultStandByAttackTimer = defaultStandByAttackTimer; }
    void SetSensing(float sensing) { mSensing = sensing; }
    void SetKnockBackSpeed(float knockBackSpeed) { mKnockBackSpeed = knockBackSpeed; }
    void SetAttackSpeed(float attackSpeed) { mAttackSpeed = attackSpeed; }

    void SetMeshes(std::vector<struct LoadedMesh>* meshes) { mMeshes = meshes; }

    bool GetIsAlive() const { return mIsAlive; }
    bool GetIsDamaged() const { return mIsDamaged; }
    bool GetIsCountered() const { return mIsCountered; }
    bool GetIsBoss() const { return mIsBoss; }
    bool GetIsBroken() const { return mIsBroken; }
    bool GetOnGround() const { return mOnGround; }
    bool GetIsStrongAttacked() const { return mIsStrongAttacked; }
    bool GetIsAttack() const { return mIsAttack; }

    int GetCurrentPlanetNum() const { return mCurrentPlanetNum; }
    int GetBreakCount() const { return mBreakCount; }
    
    float GetHp() const { return mHp; }
    float GetMaxHp() const { return mMaxHp; }
    float GetDeathTimer() const { return mDeathTimer; }
    float GetRadius() const { return mRadius; }
    float GetSpeed() const { return mSpeed; }
    float GetAttack() const { return mAttack; }
    float GetDefaultAttackMotionTimer() const { return mDefaultAttackMotionTimer; }
    float GetStandByAttackTimer() const { return mStandByAttackTimer; }
    float GetDefaultStandByAttackTimer() const { return mDefaultStandByAttackTimer; }
    float GetLaunchedTimer() const { return mLaunchedTimer; }
    float GetDefaultLaunchedTimer() const { return mDefaultLaunchedTimer; }
    float GetSensing() const { return mSensing; }
    float GetKnockBackSpeed() const { return mKnockBackSpeed; }
    float GetAttackSpeed() const { return mAttackSpeed; }

    std::vector<struct LoadedMesh>* GetMeshes() const { return mMeshes; }

private:
    void UpdateAlive(float deltaTime);
    void UpdateDying(float deltaTime);

    void UpdateKnockBack(float deltaTime, class Player* player);
    void UpdateBehavior(float deltaTime, Player* player);
    void ApplyDamage(Player* player);
    void ApplyCounter(Player* player);
    void StartDying();
    void ApplyBreak(float deltaTime);
    void LaunchCharacter(float deltaTime);
    void UpdateMotionTimer(float deltaTime, Player* player);
    void ApplyGravity(float deltaTime);
    void FixPlanetSurface();
    void FinishDying();

private:
    bool mIsAlive;
    bool mIsDamaged;
    bool mIsCountered;
    bool mIsBoss;
    bool mIsBroken;
    bool mOnGround;
    bool mIsPreparing;
    bool mIsHit;
    bool mIsStrongAttacked;
    bool mIsAttack;

    int mCurrentPlanetNum;
    int mBreakCount;
    int mBreakCountMax;

    float mRadius;
    float mSpeed;
    float mAttack;
    float mHp;
    float mMaxHp;
    float mSensing;
    float mStandByAttackTimer;
    float mDefaultStandByAttackTimer;
    float mLaunchedTimer;
    float mDefaultLaunchedTimer;
    float mAttackMotionTimer;
    float mDefaultAttackMotionTimer;
    float mDeathTimer;
    float mKnockBackTimer;
    float mKnockBackSpeed;
    float mAttackSpeed;

    glm::vec3 mVelocity;

    std::vector<struct LoadedMesh>* mMeshes;
};