#include "Planet.h"
#include "Game.h"
#include "actor/Enemy.h"
#include "actor/BoatParts.h"

Planet::Planet(Game* game)
    : Actor(game)
    , mIsAllEnemiesDead(false)
    , mIsAllBoatPartsCollected(false)
    , mStageNum(0)
    , mRemainBoatPartsCount(0)
    , mColor(1.0f)
    , mKey(nullptr)
    , mStar(nullptr)
    , mCurrentStage(nullptr)
    , mKeySpawnCondition(KeySpawnCondition::None)
    , mPlanetShape(PlanetShape::Normal)
{
}

void Planet::Initialize()
{
    
}

void Planet::UpdateActor(float deltaTime) {  
    UpdateRemainBoatPartsCount();  
    CheckKeySpawnCondition();
}

void Planet::UpdateRemainBoatPartsCount() {
    if (mBoatParts.empty()) return;

    mRemainBoatPartsCount = 0;
    for (auto parts : mBoatParts) {
        if (!parts->GetIsActive()) continue;

        mRemainBoatPartsCount++;
    }
}

void Planet::CheckKeySpawnCondition() {
    switch(mKeySpawnCondition) {
        case KeySpawnCondition::AllEnemiesDead: 
            if (mIsAllEnemiesDead) break;
            
            CheckIsAllEnemiesDead();
            break;
        
        case KeySpawnCondition::AllBoatPartsCollected: 
            if (mIsAllBoatPartsCollected) break;

            CheckIsAllBoatPartsCollected();            
            break;
        
        default:
            break;
    }
}

void Planet::CheckIsAllEnemiesDead() {
    mIsAllEnemiesDead = true;
    for (auto enemy : mEnemies) {
        if (enemy->GetIsDead()) continue;

        mIsAllEnemiesDead = false;
        return;
    }
}

void Planet::CheckIsAllBoatPartsCollected() {
    mIsAllBoatPartsCollected = true;
    for (auto parts : mBoatParts) {
        if (!parts->GetIsActive()) continue;

        mIsAllBoatPartsCollected = false;
        return;
    }
}