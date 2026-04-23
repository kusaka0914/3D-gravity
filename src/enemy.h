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
    float GetRadius() const override { return mScale * 3.0f; }

    void SetCurrentPlanet(class Planet* currentPlanet) { mCurrentPlanet = currentPlanet; }
    void SetPos(const glm::vec3& pos) { mPos = pos; }
    void SetCurrentPlanetNum(int currentPlanetNum) { mCurrentPlanetNum = currentPlanetNum; }
    void SetHp(float hp) { mHp = hp; }
    void SetIsAlive(bool isAlive) { mIsAlive = isAlive; }
    void SetIsDamaged(bool isDamaged) { mIsDamaged = isDamaged; }
    void SetIsCountered(bool isCountered) { mIsCountered = isCountered; }
    void SetIsBoss(bool isBoss) { mIsBoss = isBoss; }
    void SetIsLaunched(bool isLaunched) { mIsLaunched = isLaunched; }
    void SetIsStrongAttacked(bool isStrongAttacked) { mIsStrongAttacked = isStrongAttacked; }
    void SetDamageTimer(float damageTimer) { mDamageTimer = damageTimer; }
    void SetLaunchedTimer(float launchedTimer) { mLaunchedTimer = launchedTimer; }
    void SetModelPath(const std::string& modelPath) { mModelPath = modelPath; }
    void SetScale(float scale) { mScale = scale; }
    void SetSpeed(float speed) { mSpeed = speed; }
    void SetAttack(float attack) { mAttack = attack; }
    void SetStandByAttackTimer(float standByAttackTimer) { mStandByAttackTimer = standByAttackTimer; }
    void SetIsAttack(bool isAttack) { mIsAttack = isAttack; }
    void SetSensing(float sensing) { mSensing = sensing; }
    void SetBreakCount(int breakCount) { mBreakCount = breakCount; }
    void SetBreakCountMax(int breakCountMax) { mBreakCountMax = breakCountMax; }

    class Planet* GetCurrentPlanet() const { return mCurrentPlanet; }
    const glm::vec3& GetPos() const override { return mPos; }
    int GetCurrentPlanetNum() const { return mCurrentPlanetNum; }
    float GetHp() const { return mHp; }
    bool GetIsAlive() const { return mIsAlive; }
    bool GetIsDamaged() const { return mIsDamaged; }
    bool GetIsCountered() const { return mIsCountered; }
    bool GetIsBoss() const { return mIsBoss; }
    bool GetIsLaunched() const { return mIsLaunched; }
    bool GetOnGround() const { return mOnGround; }
    bool GetIsStrongAttacked() const { return mIsStrongAttacked; }
    float GetDamageTimer() const { return mDamageTimer; }
    const std::string& GetModelPath() const { return mModelPath; }
    float GetScale() const { return mScale; }
    float GetSpeed() const { return mSpeed; }
    float GetAttack() const { return mAttack; }
    float GetStandByAttackTimer() const { return mStandByAttackTimer; }
    float GetLaunchedTimer() const { return mLaunchedTimer; }
    bool GetIsAttack() const { return mIsAttack; }
    float GetSensing() const { return mSensing; }

private:
    bool mIsAlive;
    bool mIsDamaged;
    bool mIsCountered;
    bool mIsBoss;
    bool mIsLaunched;
    bool mOnGround;
    bool mIsPreparing;
    bool mIsHit;
    bool mIsStrongAttacked;
    bool mIsAttack;

    int mCurrentPlanetNum;
    int mBreakCount;
    int mBreakCountMax;

    float mScale;
    float mSpeed;
    float mAttack;
    float mHp;
    float mSensing;
    float mStandByAttackTimer;
    float mLaunchedTimer;
    float mAttackMotionTimer;
    float mDamageTimer;

    glm::vec3 mPos;
    glm::vec3 mVelocity;
    glm::vec3 mUpVec;

    std::string mModelPath;

    class Planet* mCurrentPlanet;
};