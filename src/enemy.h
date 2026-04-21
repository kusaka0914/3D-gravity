#pragma once

#include "Actor.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class Enemy : public Actor {
public:
    Enemy(class Game* game);
    void Initialize() override;
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
    void SetDamageTimer(float damageTimer) { mDamageTimer = damageTimer; }
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
    class Planet* mCurrentPlanet;
    glm::vec3 mPos;
    glm::vec3 mVelocity;
    glm::vec3 mUpVec;
    int mCurrentPlanetNum;
    float mHp;
    bool mIsAlive;
    bool mIsDamaged;
    bool mIsCountered;
    bool mIsBoss;
    bool mIsLaunched;
    bool mOnGround;
    bool mIsPreparing;
    bool mIsHit;
    float mDamageTimer;
    std::string mModelPath;
    float mScale;
    float mSpeed;
    float mAttack;
    float mStandByAttackTimer;
    float mLaunchedTimer;
    float mAttackMotionTimer;
    bool mIsAttack;
    float mSensing;
    int mBreakCount;
    int mBreakCountMax;
};