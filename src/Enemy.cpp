#include "Enemy.h"

Enemy::Enemy()
    :Actor(game),
    ,mPos({0.0f, 0.0f, 0.0f})
    ,mCurrentPlanet(0)
    ,mHp(10.0f)
    ,mIsAlive(true)
    ,mDamageTimer(0.0f)
    ,mModelPath("enemy.obj")
    ,mScale(0.25f)
    ,mSpeed(2.0f)
    ,mAttack(20.0f)
    ,mStandByAttackTimer(-1.0f)
    ,mIsAttack(false)
    ,mSensing(6.0f)
{
    
}