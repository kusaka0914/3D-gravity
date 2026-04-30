#include "Enemy.h"
#include "Star.h"
#include "Game.h"
#include "Planet.h"
#include "Player.h"
#include "Stage.h"
#include "GameProgressState.h"
#include "UIState.h"

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
    ,mKnockBackTimer(-1.0f)
    ,mIsBroken(false)
    ,mIsCountered(false)
{
    
}

void Enemy::UpdateActor(float deltaTime) {
    Actor::UpdateActor(deltaTime);

    if (!mIsAlive)
        return;
    
    UpdateUpVec();

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
        
        if (mOnGround) {
            FixPlanetSurface();
        } else {
            ApplyGravity(deltaTime);
        }
        mDeathTimer -= deltaTime;
        
        // 演出終了処理
        if (mDeathTimer <= 0.0f) {
            FinishDying();
        }
    }
}

void Enemy::UpdateUpVec() {
    glm::vec3 center = mCurrentPlanet->GetCenter();
    float radius = mCurrentPlanet->GetRadius();
    if (mCurrentPlanet->GetPlanetType() == Planet::PlanetType::Normal) {
        mUpVec = {0.0f, 1.0f, 0.0f};
    } else {
        mUpVec = glm::normalize(mPos - mCurrentPlanet->GetCenter());
    }
}

void Enemy::UpdateKnockBack(float deltaTime, Player* player) {
    glm::vec3 playerPos = player->GetPos();
    glm::vec3 toEnemy = glm::normalize(mPos - playerPos);
    float moveSpeed = 5.0f;
    mPos += toEnemy * moveSpeed * deltaTime;
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
        float planetRadius = mCurrentPlanet->GetRadius();
        glm::vec3 planetCenter = mCurrentPlanet->GetCenter();
        mPos = planetCenter + glm::normalize(mPos - planetCenter) * planetRadius;
    }

    // 攻撃タイマー開始
    if (inRangeOfPlayer && mStandByAttackTimer <= 0.0f && mAttackMotionTimer <= 0.0f)
    {
        if (!player->GetIsDamagePrev()) {
            mIsPreparing = true;
            mStandByAttackTimer = 2.0f;
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
        mAttackMotionTimer = 1.5f;
        mIsPreparing = false;
        mIsHit = false;
    }
}

void Enemy::ApplyDamage(Player* player) {
    mIsDamaged = false;
    mHp -= player->GetAttack();
    if (mIsStrongAttacked) {
        mLaunchedTimer = -1.0f;
        mBreakCount = mBreakCountMax;
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
        GetGame()->GetUIState()->SetCurrentTutorialKind("Break");
        GetGame()->GetGameProgressState()->SetSceneState("ShowUI");
    }
    float launchSpeed = 5.0f;
    mVelocity += mUpVec * launchSpeed;
    mPos += mVelocity * deltaTime;
    mOnGround = false;
    mStandByAttackTimer = -1.0f;
    mAttackMotionTimer = -1.0f;
    mIsPreparing = false;
    GetGame()->GetAudioSystem()->PlaySE("breakSE");
    GetGame()->SetHitStopTimer(0.6f);
}

void Enemy::UpdateMotionTimer(float deltaTime, Player* player) {
    mAttackMotionTimer -= deltaTime;

    glm::vec3 playerPos = player->GetPos();
    glm::vec3 toPlayer = glm::normalize(playerPos - mPos);
    float attackSpeed = 2.5f;
    float halfAttackMotionTimer = 0.7f;
    if (mAttackMotionTimer >= halfAttackMotionTimer) {
        mPos += toPlayer * attackSpeed * deltaTime;
    }else {
        mPos -= toPlayer * attackSpeed * deltaTime;
    }
    FixPlanetSurface();

    float distToPlayer = glm::length(playerPos - mPos);

    const float attackRangeMargin = 0.2f;
    bool inRangeOfPlayer = (distToPlayer <= GetRadius() + attackRangeMargin);
    if (inRangeOfPlayer && !mIsHit && player->GetInvincibleTimer() <= 0.0f)
    {
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

        glm::vec3 center = mCurrentPlanet->GetCenter();
        float radius = mCurrentPlanet->GetRadius();
        if (glm::length(mPos - center) <= radius) {
            mOnGround = true;
            mVelocity = {0.0f, 0.0f, 0.0f};
        }
    } else {
        mLaunchedTimer -= deltaTime;
        if (mLaunchedTimer <= 0.0f) {
            mBreakCount = mBreakCountMax;
        }
        return;
    }
    
    float vPrev = dot(prevVelocity, mUpVec);
    float vNow  = dot(mVelocity, mUpVec);

    // 頂点で固定開始
    if (vPrev > 0.0f && vNow <= 0.0f) {
        mLaunchedTimer = 3.0f;
    }
}

void Enemy::FixPlanetSurface() {
    glm::vec3 center = mCurrentPlanet->GetCenter();
    float radius = mCurrentPlanet->GetRadius();
    mPos = center + glm::normalize(mPos - center) * radius;
}

void Enemy::FinishDying() {
    mIsAlive = false;
    // ボス撃破でスター出現（撃破前のボスがいた場所に置く）
    Star* star = mCurrentPlanet->GetStar();
    if (mIsBoss)
    {
        star->SetIsActive(true); 
        star->SetPos(mPos);
    }
}