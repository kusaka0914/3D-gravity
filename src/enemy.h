#ifndef ENEMY_H
#define ENEMY_H

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class Enemy : Actor {
public:
    Enemy(class Game* game);
    void UpdateActor(float deltaTime) override;

    virtual bool IsBoss() const { return false; }
    float GetRadius() const { return mScale * 3.6f; }

    void SetPos(const glm::vec3& pos) { mPos = pos; }
    void SetCurrentPlanet(int currentPlanet) { mCurrentPlanet = currentPlanet; }
    void SetHp(float hp) { mHp = hp; }
    void SetIsAlive(bool isAlive) { mIsAlive = isAlive; }
    void SetDamageTimer(float damageTimer) { mDamageTimer = damageTimer; }
    void SetModelPath(const std::string& modelPath) { mModelPath = modelPath; }
    void SetScale(float scale) { mScale = scale; }
    void SetSpeed(float speed) { mSpeed = speed; }
    void SetAttack(float attack) { mAttack = attack; }
    void SetStandByAttackTimer(float standByAttackTimer) { mStandByAttackTimer = standByAttackTimer; }
    void SetIsAttack(bool isAttack) { mIsAttack = isAttack; }
    void SetSensing(float sensing) { mSensing = sensing; }

    void SetPos(const glm::vec3& pos) { mPos = pos; }
    void SetCurrentPlanet(int currentPlanet) { mCurrentPlanet = currentPlanet; }
    void SetHp(float hp) { mHp = hp; }
    void SetIsAlive(bool isAlive) { mIsAlive = isAlive; }
    void SetDamageTimer(float damageTimer) { mDamageTimer = damageTimer; }
    void SetModelPath(const std::string& modelPath) { mModelPath = modelPath; }
    void SetScale(float scale) { mScale = scale; }
    void SetSpeed(float speed) { mSpeed = speed; }
    void SetAttack(float attack) { mAttack = attack; }
    void SetStandByAttackTimer(float standByAttackTimer) { mStandByAttackTimer = standByAttackTimer; }
    void SetIsAttack(bool isAttack) { mIsAttack = isAttack; }
    void SetSensing(float sensing) { mSensing = sensing; }

    const glm::vec3& GetPos() const { return mPos; }
    int GetCurrentPlanet() const { return mCurrentPlanet; }
    float GetHp() const { return mHp; }
    bool GetIsAlive() const { return mIsAlive; }
    float GetDamageTimer() const { return mDamageTimer; }
    const std::string& GetModelPath() const { return mModelPath; }
    float GetScale() const { return mScale; }
    float GetSpeed() const { return mSpeed; }
    float GetAttack() const { return mAttack; }
    float GetStandByAttackTimer() const { return mStandByAttackTimer; }
    bool GetIsAttack() const { return mIsAttack; }
    float GetSensing() const { return mSensing; }

private:
    glm::vec3 mPos;
    int mCurrentPlanet;
    float mHp;
    bool mIsAlive;
    float mDamageTimer;
    std::string mModelPath;
    float mScale;
    float mSpeed;
    float mAttack;
    float mStandByAttackTimer;
    bool mIsAttack;
    float mSensing;
};

class NormalEnemy : public EnemyBase {
public:
    bool isBoss() const override { return false; }
};

class BossEnemy : public EnemyBase {
public:
    bool isBoss() const override { return true; }
};

#endif
