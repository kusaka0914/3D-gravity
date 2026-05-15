#include "Planet.h"
#include "Game.h"
#include "actor/Enemy.h"
#include "actor/BoatParts.h"

Planet::Planet(Game* game)
    : Actor(game)
    , mColor(1.0f)
    , mKey(nullptr)
    , mIsAllEnemiesDead(false)
    , mIsAllBoatPartsCollected(false)
    , mStar(nullptr)
{
    
}

void Planet::Initialize()
{
    
}

void Planet::UpdateActor(float deltaTime) {    
    CheckKeySpawnCondition();
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