#ifndef ENEMY_H
#define ENEMY_H

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class Enemy : Actor {
public:
    Enemy(class Game* game);

    float getRadius() const { return drawScale * 3.6f; }

    virtual bool isBoss() const { return false; }
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
