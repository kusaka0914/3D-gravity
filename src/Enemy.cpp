#include "Enemy.h"
#include "Star.h"
#include "Game.h"
#include "Planet.h"
#include "Player.h"
#include "Stage.h"

Enemy::Enemy(Game* game)
    :Actor(game)
    ,mPos({0.0f, 8.0f, 0.0f})
    ,mVelocity(0.0f)
    ,mCurrentPlanetNum(0)
    ,mHp(10.0f)
    ,mIsAlive(true)
    ,mOnGround(true)
    ,mDeathTimer(-1.0f)
    ,mIsDamaged(false)
    ,mModelPath("enemy.obj")
    ,mScale(0.25f)
    ,mSpeed(2.0f)
    ,mAttack(20.0f)
    ,mStandByAttackTimer(-1.0f)
    ,mLaunchedTimer(-1.0f)
    ,mIsAttack(false)
    ,mSensing(6.0f)
    ,mIsStrongAttacked(false)
    ,mAttackMotionTimer(-1.0f)
    ,mIsBroken(false)
    ,mIsCountered(false)
{
    
}

void Enemy::UpdateActor(float deltaTime) {
    Actor::UpdateActor(deltaTime);
    std::vector<Player*> players = GetGame()->GetPlayers();
    
    if (mIsAlive && mDeathTimer <= 0.0f) {
        for(auto player : players) {
            glm::vec3 playerPos = player->GetPos();
            float distToPlayer = glm::length(playerPos - mPos);
            glm::vec3 vecToPlayer = glm::normalize(playerPos - mPos);

            const float attackRangeMargin = 0.2f;
            bool inRangeOfPlayer = (distToPlayer <= GetRadius() + attackRangeMargin);

            // 敵AIの実装
            if(mOnGround) {
                // 追跡
                if (distToPlayer <= mSensing && !player->GetIsDamaged() && distToPlayer >= GetRadius() + 0.2f && mStandByAttackTimer <= 0.0f && mAttackMotionTimer <= 0.0f)
                {
                    mPos += vecToPlayer * mSpeed * deltaTime;
                    float planetRadius = mCurrentPlanet->GetRadius();
                    glm::vec3 planetCenter = mCurrentPlanet->GetCenter();
                    mPos = planetCenter + glm::normalize(mPos - planetCenter) * planetRadius;
                }

                // 攻撃タイマー開始
                if (inRangeOfPlayer && mStandByAttackTimer <= 0.0f)
                {
                    if (!player->GetIsDamagePrev()) {
                        mIsPreparing = true;
                        mStandByAttackTimer = 1.0f;
                    }
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
                if (mStandByAttackTimer <= 0.0f && mIsPreparing)
                {
                    mStandByAttackTimer = -1.0f;
                    mAttackMotionTimer = 1.0f;
                    mIsPreparing = false;
                    mIsHit = false;
                }
            }

            if (mIsDamaged) {
                if (mIsStrongAttacked) {
                    mHp -= player->GetAttack() * 5;
                    GetGame()->SetHitStopTimer(0.6f);
                    GetGame()->GetAudioSystem()->PlaySE("attackAirSE");
                    mIsStrongAttacked = false;
                } else {
                    mHp -= player->GetAttack();
                }
                mIsDamaged = false;
            }

            if (mIsCountered) {
                mStandByAttackTimer = -1.0f; // 攻撃準備状態を解除
                mDeathTimer = 0.3f;
                mHp -= player->GetAttack() * 2.0f;
                mIsCountered = false;
            }

            if (mHp <= 0) {
                mDeathTimer = 1.0f; // 死亡時演出を開始する
                mHp = 0;
                GetGame()->GetAudioSystem()->PlaySE("defeatSE");
            }

            // TODO: 敵にもレイキャストでの当たり判定を実装
            glm::vec3 center = mCurrentPlanet->GetCenter();
            float radius = mCurrentPlanet->GetRadius();
            
            mUpVec = glm::normalize(mPos - center);
            if (mIsBroken) {
                mIsBroken = false;
                mBreakCount--;

                // 完全破壊時に空中に打ち上げる
                if (mBreakCount <= 0) {
                    mVelocity += mUpVec * 5.0f;
                    mOnGround = false;
                    mStandByAttackTimer = -1.0f;
                    mIsPreparing = false;
                    mBreakCount = mBreakCountMax;
                    GetGame()->GetAudioSystem()->PlaySE("breakSE");
                    GetGame()->SetHitStopTimer(0.6f);
                } else {
                    GetGame()->GetAudioSystem()->PlaySE("destroySE");
                }
            }

            // 重力処理
            glm::vec3 prevVelocity = mVelocity;
            if (mLaunchedTimer <= 0.0f && !mOnGround) {
                mVelocity -= mUpVec * 9.8f * deltaTime;
                mPos += mVelocity * deltaTime;
                if (glm::length(mPos - center) <= radius) {
                    mOnGround = true;
                    mVelocity = {0.0f, 0.0f, 0.0f};
                }
            } else {
                mLaunchedTimer -= deltaTime;
            }
            
            float vPrev = dot(prevVelocity, mUpVec);
            float vNow  = dot(mVelocity, mUpVec);

            // 頂点で固定開始
            if (vPrev > 0.0f && vNow <= 0.0f) {
                mLaunchedTimer = 2.0f;
            }

            // 攻撃モーション中の移動処理
            if (mAttackMotionTimer >= 0.0f)
            {
                mAttackMotionTimer -= deltaTime;
                glm::vec3 toPlayer = glm::normalize(player->GetPos() - mPos);
                if (mAttackMotionTimer >= 0.3f) {
                    mPos += toPlayer * 2.5f * deltaTime;
                }else {
                    mPos -= toPlayer * 2.5f * deltaTime;
                }
                mPos = center + glm::normalize(mPos - center) * radius;
                if (inRangeOfPlayer && !mIsHit)
                {
                    player->SetIsDamaged(true);
                    player->SetHp(player->GetHp() - mAttack);
                    player->SetKnockBackFrom(mPos);
                    mIsHit = true;
                }
            }
        }
    }
    // 死亡時演出処理 
    else if (mIsAlive && mDeathTimer > 0.0f) {
        for(auto player : players) {
            glm::vec3 toEnemy = glm::normalize(mPos - player->GetPos());
            mPos += toEnemy * 3.0f * deltaTime;
            glm::vec3 center = mCurrentPlanet->GetCenter();
            float radius = mCurrentPlanet->GetRadius();
            if (!mOnGround) {
                mVelocity -= mUpVec * 9.8f * deltaTime;
                mPos += mVelocity * deltaTime;
                if (glm::length(mPos - center) <= radius) {
                    mOnGround = true;
                    mVelocity = {0.0f, 0.0f, 0.0f};
                }
            } else {
                mPos = mCurrentPlanet->GetCenter() + glm::normalize(mPos - mCurrentPlanet->GetCenter()) * mCurrentPlanet->GetRadius();
            }
            mDeathTimer -= deltaTime;
            
            // 演出終了処理
            if (mDeathTimer <= 0.0f) {
                mIsAlive = false;
                // ボス撃破でスター出現（撃破前のボスがいた場所に置く）
                Star* star = mCurrentPlanet->GetStar();
                if (mIsBoss)
                {
                    star->SetIsActive(true);
                    star->SetCurrentPlanet(mCurrentPlanetNum);
                    star->SetPos(mPos);
                }
            }
        }
    }
}