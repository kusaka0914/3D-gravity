#include "Planet.h"
#include "Game.h"
#include "actor/BoatParts.h"
#include "actor/Enemy.h"

Planet::Planet(Game* game)
    : Actor(game),
      mIsAllEnemiesDead(false),
      mIsAllBoatPartsCollected(false),
      mStageNum(0),
      mRemainBoatPartsCount(0),
      mColor(1.0f),
      mKey(nullptr),
      mStar(nullptr),
      mCurrentStage(nullptr),
      mRocketSpawnCondition(RocketSpawnCondition::None),
      mPlanetShape(PlanetShape::Normal)
{
}

void Planet::Initialize()
{
    InitRemainBoatPartsCount();
}

void Planet::InitRemainBoatPartsCount()
{
    if (mBoatParts.empty()) {
        return;
    }

    mRemainBoatPartsCount = 0;
    for (auto parts : mBoatParts) {
        if (!parts->GetIsActive()) {
            continue;
        }

        mRemainBoatPartsCount++;
    }
}

void Planet::OnEnemyDead()
{
    bool shouldCheckIsAllEnemiesDead =
        mRocketSpawnCondition == RocketSpawnCondition::AllEnemiesDead && !mIsAllEnemiesDead;
    if (shouldCheckIsAllEnemiesDead) {
        CheckIsAllEnemiesDead();
    }
}

void Planet::CheckIsAllEnemiesDead()
{
    mIsAllEnemiesDead = true;
    for (auto enemy : mEnemies) {
        if (enemy->GetIsDead()) {
            continue;
        }

        mIsAllEnemiesDead = false;
        return;
    }
}

void Planet::OnBoatPartsObtained()
{
    mRemainBoatPartsCount--;

    bool shouldCheckAllBoatPartsCollected =
        mRocketSpawnCondition == RocketSpawnCondition::AllBoatPartsCollected && !mIsAllBoatPartsCollected;
    if (shouldCheckAllBoatPartsCollected) {
        CheckIsAllBoatPartsCollected();
    }
}

void Planet::CheckIsAllBoatPartsCollected()
{
    mIsAllBoatPartsCollected = true;
    for (auto parts : mBoatParts) {
        if (!parts->GetIsActive()) {
            continue;
        }

        mIsAllBoatPartsCollected = false;
        return;
    }
}