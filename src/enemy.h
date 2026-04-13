#ifndef ENEMY_H
#define ENEMY_H

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
    float GetRadius() const { return mScale * 3.6f; }

    void SetCurrentPlanet(class Planet* currentPlanet) { mCurrentPlanet = currentPlanet; }
    void SetPos(const glm::vec3& pos) { mPos = pos; }
    void SetCurrentPlanetNum(int currentPlanetNum) { mCurrentPlanetNum = currentPlanetNum; }
    void SetHp(float hp) { mHp = hp; }
    void SetIsAlive(bool isAlive) { mIsAlive = isAlive; }
    void SetIsDamaged(bool isDamaged) { mIsDamaged = isDamaged; }
    void SetIsCountered(bool isCountered) { mIsCountered = isCountered; }
    void SetDamageTimer(float damageTimer) { mDamageTimer = damageTimer; }
    void SetModelPath(const std::string& modelPath) { mModelPath = modelPath; }
    void SetScale(float scale) { mScale = scale; }
    void SetSpeed(float speed) { mSpeed = speed; }
    void SetAttack(float attack) { mAttack = attack; }
    void SetStandByAttackTimer(float standByAttackTimer) { mStandByAttackTimer = standByAttackTimer; }
    void SetIsAttack(bool isAttack) { mIsAttack = isAttack; }
    void SetSensing(float sensing) { mSensing = sensing; }

    class Planet* GetCurrentPlanet() const { return mCurrentPlanet; }
    const glm::vec3& GetPos() const { return mPos; }
    int GetCurrentPlanetNum() const { return mCurrentPlanetNum; }
    float GetHp() const { return mHp; }
    bool GetIsAlive() const { return mIsAlive; }
    bool GetIsDamaged() const { return mIsDamaged; }
    bool GetIsCountered() const { return mIsCountered; }
    float GetDamageTimer() const { return mDamageTimer; }
    const std::string& GetModelPath() const { return mModelPath; }
    float GetScale() const { return mScale; }
    float GetSpeed() const { return mSpeed; }
    float GetAttack() const { return mAttack; }
    float GetStandByAttackTimer() const { return mStandByAttackTimer; }
    bool GetIsAttack() const { return mIsAttack; }
    float GetSensing() const { return mSensing; }

private:
    class Planet* mCurrentPlanet;
    glm::vec3 mPos;
    int mCurrentPlanetNum;
    float mHp;
    bool mIsAlive;
    bool mIsDamaged;
    bool mIsCountered;
    float mDamageTimer;
    std::string mModelPath;
    float mScale;
    float mSpeed;
    float mAttack;
    float mStandByAttackTimer;
    bool mIsAttack;
    float mSensing;
};

#endif
