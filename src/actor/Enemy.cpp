#include "Enemy.h"
#include "actor/Star.h"
#include "Game.h"
#include "actor/Planet.h"
#include "actor/Player.h"
#include "Stage.h"
#include "state/GameProgressState.h"
#include "state/UIState.h"
#include "system/PhysicsSystem.h"
#include <btBulletDynamicsCommon.h>

Enemy::Enemy(Game* game)
    :CharacterActor(game)
    ,mCurrentPlanetNum(0)
    ,mHp(10.0f)
    ,mIsAlive(true)
    ,mDeathTimer(-1.0f)
    ,mIsDamaged(false)
    ,mSpeed(2.0f)
    ,mAttack(20.0f)
    ,mStandByAttackTimer(-1.0f)
    ,mLaunchedTimer(-1.0f)
    ,mIsAttack(false)
    ,mSensing(6.0f)
    ,mIsStrongAttacked(false)
    ,mAttackMotionTimer(-1.0f)
    ,mKnockBackTimer(-1.0f)
    ,mIsBroken(false)
    ,mIsCountered(false)
    ,mDefaultStandByAttackTimer(-1.0f)
    ,mDefaultLaunchedTimer(-1.0f)
    ,mDefaultAttackMotionTimer(-1.0f)
    ,mKnockBackSpeed(5.0f)
    ,mAttackSpeed(1.5f)
{
    
}

void Enemy::UpdateActor(float deltaTime) {
    Actor::UpdateActor(deltaTime);

    if (!mIsAlive) return;

    if (mDeathTimer > 0.0f) {
        UpdateDying(deltaTime);
        return;
    } 

    UpdateAlive(deltaTime); 
}

void Enemy::UpdateAlive(float deltaTime) {
    std::vector<Player*> players = GetGame()->GetPlayers();
    for(auto player : players) {
        if (mIsDamaged) {
            ApplyDamage(player);
        }

        if (mIsCountered) {
            ApplyCounter(player);
        }

        if (mHp <= 0) {
            StartDying();
            return;
        }

        // TODO: 敵にもレイキャストでの当たり判定を実装
        if (mIsBroken) {
            ApplyBreak(deltaTime);
            return;
        }

        if(mOnGround) {
            UpdateBehavior(deltaTime, player);
        } else {
            ApplyGravity(deltaTime);
        }

        // 攻撃モーション中の移動処理
        if (mAttackMotionTimer >= 0.0f)
        {
            UpdateMotionTimer(deltaTime, player);
        }

        if (mKnockBackTimer >= 0.0f) {
            UpdateKnockBack(deltaTime, player);
        }
    }
}

void Enemy::UpdateDying(float deltaTime) {
    std::vector<Player*> players = GetGame()->GetPlayers();
    for(auto player : players) {
        UpdateKnockBack(deltaTime, player);
        
        if (!mOnGround) {
            ApplyGravity(deltaTime);
        }
        mDeathTimer -= deltaTime;
        
        // 演出終了処理
        if (mDeathTimer <= 0.0f) {
            FinishDying();
        }
    }
}

void Enemy::UpdateKnockBack(float deltaTime, Player* player) {
    glm::vec3 playerPos = player->GetPos();
    glm::vec3 toEnemy = glm::normalize(mPos - playerPos);
    mPos += toEnemy * mKnockBackSpeed * deltaTime;
    if (mKnockBackTimer >= 0.0f) {
        mKnockBackTimer -= deltaTime;
    }
}

void Enemy::UpdateBehavior(float deltaTime, Player* player) {
    glm::vec3 playerPos = player->GetPos();
    float distToPlayer = glm::length(playerPos - mPos);
    glm::vec3 vecToPlayer = glm::normalize(playerPos - mPos);

    const float attackRangeMargin = 1.5f;
    bool inRangeOfPlayer = (distToPlayer <= GetRadius() + attackRangeMargin);
    // 追跡
    if (distToPlayer <= mSensing && !player->GetIsDamaged() && distToPlayer >= GetRadius() + 0.2f && mStandByAttackTimer <= 0.0f && mAttackMotionTimer <= 0.0f && mKnockBackTimer <= 0.0f)
    {
        mPos += vecToPlayer * mSpeed * deltaTime;
    }

    // 攻撃タイマー開始
    if (inRangeOfPlayer && mStandByAttackTimer <= 0.0f && mAttackMotionTimer <= 0.0f)
    {
        if (!player->GetIsDamagePrev()) {
            mIsPreparing = true;
            mStandByAttackTimer = mDefaultStandByAttackTimer;
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
        mAttackMotionTimer = mDefaultAttackMotionTimer;
        mIsPreparing = false;
        mIsHit = false;
    }
}

void Enemy::ApplyDamage(Player* player) {
    mIsDamaged = false;
    mHp -= player->GetAttack();
    if (mIsStrongAttacked) {
        FinishLaunched();
        GetGame()->SetHitStopTimer(0.6f);
        mIsStrongAttacked = false;
        mKnockBackTimer = 1.0f;
    }
}

void Enemy::ApplyCounter(Player* player) {
    mIsCountered = false;
    mHp -= player->GetAttack() * 2.0f;
    mStandByAttackTimer = -1.0f; // 攻撃準備状態を解除  
    mKnockBackTimer = 0.6f;      
}

void Enemy::StartDying() {
    mDeathTimer = 1.0f; // 死亡時演出を開始する
    mHp = 0;
    GetGame()->GetAudioSystem()->PlaySE("defeatSE");
}

void Enemy::ApplyBreak(float deltaTime) {
    mIsBroken = false;
    mBreakCount--;

    if (mBreakCount <= 0) {
        LaunchCharacter(deltaTime);
        return;
    }

    GetGame()->GetAudioSystem()->PlaySE("destroySE");
}

void Enemy::LaunchCharacter(float deltaTime) {
    if (!GetGame()->GetGameProgressState()->GetIsFirstBreak()) {
        GetGame()->GetGameProgressState()->SetIsFirstBreak(true);
        GetGame()->GetUIState()->SetCurrentTutorialKind(UIState::TutorialKind::Break);
        GetGame()->GetGameProgressState()->SetCurrentSceneState(GameProgressState::SceneState::Talking);
    }
    float launchSpeed = 5.0f;
    mVelocity += mUpVec * launchSpeed;
    mPos += mVelocity * deltaTime;
    mOnGround = false;
    mStandByAttackTimer = -1.0f;
    mAttackMotionTimer = -1.0f;
    mIsPreparing = false;
    mIsJudgeLanding = false;
    GetGame()->GetAudioSystem()->PlaySE("breakSE");
    GetGame()->SetHitStopTimer(0.6f);
}

void Enemy::FinishLaunched() {
    mBreakCount = mBreakCountMax;
    mIsJudgeLanding = true;
    mLaunchedTimer = -1.0f;
}

void Enemy::UpdateMotionTimer(float deltaTime, Player* player) {
    mAttackMotionTimer -= deltaTime;

    glm::vec3 playerPos = player->GetPos();
    glm::vec3 toPlayer = glm::normalize(playerPos - mPos);
    if (mAttackMotionTimer >= mDefaultAttackMotionTimer / 2) {
        mPos += toPlayer * mAttackSpeed * deltaTime;
    }else {
        mPos -= toPlayer * mAttackSpeed * deltaTime;
    }

    float distToPlayer = glm::length(playerPos - mPos);

    const float attackRangeMargin = 0.2f;
    bool inRangeOfPlayer = (distToPlayer <= GetRadius() + attackRangeMargin);
    if (inRangeOfPlayer && !mIsHit && player->GetInvincibleTimer() <= 0.0f) {
        player->SetIsDamaged(true);
        player->SetHp(player->GetHp() - mAttack);
        player->SetKnockBackFrom(mPos);
        mIsHit = true;
    }
}

void Enemy::ApplyGravity(float deltaTime) {
    glm::vec3 prevVelocity = mVelocity;
    if (mLaunchedTimer <= 0.0f) {
        float gravity = 9.8f;
        mVelocity -= mUpVec * gravity * deltaTime;
        mPos += mVelocity * deltaTime;
    } else {
        mLaunchedTimer -= deltaTime;
        if (mLaunchedTimer <= 0.0f) {
            FinishLaunched();
        }
        return;
    }
    
    float vPrev = dot(prevVelocity, mUpVec);
    float vNow  = dot(mVelocity, mUpVec);

    // 頂点で固定開始
    if (vPrev > 0.0f && vNow <= 0.0f) {
        mLaunchedTimer = mDefaultLaunchedTimer;
    }
}

void Enemy::FinishDying() {
    mIsAlive = false;
    // ボス撃破でスター出現（撃破前のボスがいた場所に置く）
    Star* star = GetCurrentPlanet()->GetStar();
    if (mIsBoss) {
        star->SetIsActive(true); 
        star->SetPos(mPos);
    }
}