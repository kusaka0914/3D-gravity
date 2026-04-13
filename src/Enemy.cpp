#include "Enemy.h"
#include "Game.h"
#include "Planet.h"
#include "Player.h"
#include "Stage.h"

Enemy::Enemy(Game* game)
    :Actor(game)
    ,mPos({0.0f, 8.0f, 0.0f})
    ,mCurrentPlanetNum(0)
    ,mHp(10.0f)
    ,mIsAlive(true)
    ,mDamageTimer(-1.0f)
    ,mModelPath("enemy.obj")
    ,mScale(0.25f)
    ,mSpeed(2.0f)
    ,mAttack(20.0f)
    ,mStandByAttackTimer(-1.0f)
    ,mIsAttack(false)
    ,mSensing(6.0f)
{
    
}

void Enemy::Initialize()
{
    
}

void Enemy::UpdateActor(float deltaTime) {
    Actor::UpdateActor(deltaTime);
    std::vector<Player*> players = GetGame()->GetPlayers();
    
    if (mIsAlive) {
        for(auto player : players) {
            glm::vec3 playerPos = player->GetPos();
            float distToPlayer = glm::length(playerPos - mPos);
            glm::vec3 vecToPlayer = glm::normalize(playerPos - mPos);

            Stage* currentStage = GetGame()->GetCurrentStage();
            Planet* currentPlanet = currentStage->GetPlanets()[mCurrentPlanetNum];

            // 追跡
            if (distToPlayer <= mSensing && mDamageTimer <= 0.0f && !player->GetIsDamaged() && distToPlayer >= GetRadius())
            {
                mPos += vecToPlayer * mSpeed * deltaTime;
                float planetRadius = currentPlanet->GetRadius();
                glm::vec3 planetCenter = currentPlanet->GetCenter();
                mPos = planetCenter + glm::normalize(mPos - planetCenter) * planetRadius;
            }

            const float attackRangeMargin = 0.2f;
            bool inRangeOfPlayer = (distToPlayer <= GetRadius() + attackRangeMargin);
            // 攻撃タイマー開始
            if (inRangeOfPlayer)
            {
                if (!player->GetIsDamagePrev())
                    mStandByAttackTimer = 2.0f;
            }
            // 攻撃準備
            if (mStandByAttackTimer >= 0.0f)
            {
                float prevStandBy = mStandByAttackTimer;
                mStandByAttackTimer -= deltaTime;
                if(prevStandBy >= 0.5f && mStandByAttackTimer <= 0.5f)
                    GetGame()->GetAudioSystem()->PlaySE("attackPreSE");
            }
            // 攻撃
            if (mStandByAttackTimer <= 0.0f && inRangeOfPlayer)
            {
                mStandByAttackTimer = -1.0f;
                if (!player->GetIsDamagePrev())
                {
                    player->SetHp(player->GetHp() - mAttack);
                    player->SetDamageTimer(1.0f);
                    player->SetIsDamaged(true);
                    player->SetKnockBackFrom(mPos);
                    if (player->GetHp() <= 0)
                    {
                        player->SetHp(0);
                        player->SetPos(player->GetRestartPos());
                        player->SetCurrentPlanetNum(player->GetRestartPlanetIndex());
                        player->SetVelocity({0.0f, 0.0f, 0.0f});
                        player->SetOnGround(true);
                    }
                }
            }

            if (mIsDamaged) {
                mHp -= player->GetAttack();
                if (mHp <= 0) {
                    mDamageTimer = 1.0f;
                    mHp = 0;
                }
                mIsDamaged = false;
            }

            if (mIsCountered) {
                mStandByAttackTimer = -1.0f;
                mDamageTimer = 1.0f;
                mHp -= player->GetAttack() * 2.0f;
                if (mHp <= 0)
                    mHp = 0;
            }
        
            if (mDamageTimer > 0.0f)
            {
                glm::vec3 toEnemy = glm::normalize(mPos - player->GetPos());
                mPos += toEnemy * deltaTime;
                mPos = mCurrentPlanet->GetCenter() + glm::normalize(mPos - mCurrentPlanet->GetCenter()) * mCurrentPlanet->GetRadius();
                mDamageTimer -= deltaTime;
            }
            else
            {
                if (mHp <= 0)
                {
                    mHp = 0;
                    mIsAlive = false;
                    // // 現在の惑星で倒した敵の位置を記録（鍵出現位置に使う）
                    // if (e.planetIndex == mPlayers[0].planetIndex)
                    //     lastDefeatedEnemyPos = mPos();
                    // // ボス撃破でスター出現（撃破前のボスがいた場所に置く）
                    // if (e.isBoss() && !starVisibleFromBoss &&
                    //     e.planetIndex >= 0 && static_cast<size_t>(e.planetIndex) < planets.size())
                    // {
                    //     starVisibleFromBoss = true;
                    //     starBossPlanetIndex = e.planetIndex;
                    //     starPos = mPos(); // ボスがいた場所
                    // }
                }
            }
        }
    }
}