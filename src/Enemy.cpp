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
    ,mDamageTimer(-1.0f)
    ,mModelPath("enemy.obj")
    ,mScale(0.25f)
    ,mSpeed(2.0f)
    ,mAttack(20.0f)
    ,mStandByAttackTimer(-1.0f)
    ,mLaunchedTimer(-1.0f)
    ,mIsAttack(false)
    ,mSensing(6.0f)
    ,mIsStrongAttacked(false)
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

            const float attackRangeMargin = 0.2f;
            bool inRangeOfPlayer = (distToPlayer <= GetRadius() + attackRangeMargin);

            if(mOnGround && mDamageTimer <= 0.0f) {
                // 追跡
                if (distToPlayer <= mSensing && mDamageTimer <= 0.0f && !player->GetIsDamaged() && distToPlayer >= GetRadius() + 0.2f && mStandByAttackTimer <= 0.0f && mAttackMotionTimer <= 0.0f)
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
                if (mHp <= 0) {
                    mDamageTimer = 1.0f;
                    mHp = 0;
                    GetGame()->GetAudioSystem()->PlaySE("defeatSE");
                }
                mIsDamaged = false;
            }

            if (mIsCountered) {
                mStandByAttackTimer = -1.0f;
                mDamageTimer = 0.3f;
                mHp -= player->GetAttack() * 2.0f;
                if (mHp <= 0)
                    mHp = 0;
                    GetGame()->GetAudioSystem()->PlaySE("defeatSE");
                mIsCountered = false;
            }

            glm::vec3 center = mCurrentPlanet->GetCenter();
            float radius = mCurrentPlanet->GetRadius();
            if (mOnGround) {
                mPos = center + glm::normalize(mPos - center) * radius;
            }

            mUpVec = glm::normalize(mPos - center);
            if (mIsLaunched) {
                mBreakCount--;
                mIsLaunched = false;
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
            if (mLaunchedTimer <= 0.0f) {
                mPos += mVelocity * deltaTime;
                mVelocity -= mUpVec * 9.8f * deltaTime;
            } 
            float vPrev = dot(prevVelocity, mUpVec);
            float vNow  = dot(mVelocity, mUpVec);

            if (vPrev > 0.0f && vNow <= 0.0f) {
                mLaunchedTimer = 2.0f;
            }

            if (mLaunchedTimer >= 0.0f) {
                mLaunchedTimer -= deltaTime;
            }

            if (mAttackMotionTimer >= 0.0f)
            {
                mAttackMotionTimer -= deltaTime;
                glm::vec3 toPlayer = glm::normalize(player->GetPos() - mPos);
                if (mAttackMotionTimer >= 0.3f) {
                    mPos += toPlayer * 2.5f * deltaTime;
                }else {
                    mPos -= toPlayer * 2.5f * deltaTime;
                }
                if (!player->GetIsDamagePrev() && inRangeOfPlayer && !mIsHit)
                {
                    GetGame()->GetAudioSystem()->PlaySE("damagedSE");
                    player->SetHp(player->GetHp() - mAttack);
                    player->SetDamageTimer(1.0f);
                    player->SetIsDamaged(true);
                    player->SetKnockBackFrom(mPos);
                    mIsHit = true;
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

            // 落下時に初期位置に移動
            float dist = glm::length(mPos - center);
            if (dist <= radius) {
                mOnGround = true;
                mVelocity = glm::vec3(0, 0, 0);
            }
        
            if (mDamageTimer > 0.0f)
            {
                glm::vec3 toEnemy = glm::normalize(mPos - player->GetPos());
                mPos += toEnemy * 3.0f * deltaTime;
                mLaunchedTimer = 0.0f;
                mPos += mVelocity * deltaTime;
                mVelocity -= mUpVec * 28.4f * deltaTime;
                mDamageTimer -= deltaTime;
            }
            else
            {
                if (mHp <= 0)
                {
                    mHp = 0;
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
}