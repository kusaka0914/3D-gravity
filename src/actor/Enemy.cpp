#include "Enemy.h"
#include "actor/Star.h"
#include "Game.h"
#include "utils/MathUtils.h"
#include "actor/Planet.h"
#include "actor/Player.h"
#include "Stage.h"
#include "state/UIState.h"
#include "system/SceneSystem.h"
#include "system/PhysicsSystem.h"
#include "system/AudioSystem.h"
#include <btBulletDynamicsCommon.h>

Enemy::Enemy(Game* game)
    :CharacterActor(game)
    , mHp(10.0f)
    , mDyingTimer(-1.0f)
    , mMoveSpeed(2.0f)
    , mAttack(20.0f)
    , mStandByAttackTimer(-1.0f)
    , mLaunchedTimer(-1.0f)
    , mDetectionRange(6.0f)
    , mIsStrongAttacked(false)
    , mAttackMotionTimer(-1.0f)
    , mKnockBackTimer(-1.0f)
    , mIsCountered(false)
    , mDefaultStandByAttackTimer(-1.0f)
    , mDefaultLaunchedTimer(-1.0f)
    , mDefaultAttackMotionTimer(-1.0f)
    , mKnockBackSpeed(5.0f)
    , mAttackSpeed(1.5f)
    , mLifeState(LifeState::Alive)
    , mActionState(ActionState::Idle)
{
    
}

void Enemy::UpdateActor(float deltaTime) {
    CharacterActor::UpdateActor(deltaTime);
    bool isPlaying = mGame->GetSceneSystem()->IsPlaying();
    if (!isPlaying) return;
    
    switch (mLifeState) {
        case LifeState::Alive:
            UpdateAlive(deltaTime); 
            break;
        
        case LifeState::Dying:
            UpdateDying(deltaTime);
            break;

        case LifeState::Dead:
            break;
    }
}

void Enemy::UpdateAlive(float deltaTime) {
    std::vector<Player*> players = mGame->GetPlayers();
    for(auto player : players) {
        if(mOnGround)
            UpdateBehavior(deltaTime, player);
        else 
            UpdateInAir(deltaTime);
    }
}

void Enemy::UpdateDying(float deltaTime) {
    std::vector<Player*> players = mGame->GetPlayers();
    for(auto player : players) {
        MoveDuringKnockBack(deltaTime, player);
        
        if (!mOnGround)
            UpdateInAir(deltaTime);
            
        mDyingTimer -= deltaTime;
        if (mDyingTimer <= 0.0f)
            FinishDying();
    }
}

void Enemy::UpdateBehavior(float deltaTime, Player* player) {
    switch (mActionState) {
        case ActionState::Idle:
            UpdateIdle(player);
            break;
            
        case ActionState::Tracking:
            UpdateTracking(deltaTime, player);
            break;

        case ActionState::PreparingAttack: 
            UpdatePreparingAttack(deltaTime);
            break;

        case ActionState::Attacking:
            UpdateAttacking(deltaTime, player);
            break;

        case ActionState::KnockedBack:
            UpdateKnockedBack(deltaTime, player);
            break;
    }
}

void Enemy::UpdateFacingVec() {
    Player* player = mGame->GetPlayers()[0];
    glm::vec3 toPlayer = glm::normalize(player->GetPos() - mPos);

    mFacingForwardVec = toPlayer;
    mFacingYaw = mGame->GetMathUtils()->GetYawFromDirection(mUpVec, toPlayer) + 3.14159265f;
}

void Enemy::UpdateIdle(Player* player) {
    if (IsPlayerInRange(mDetectionRange, player))
        StartTracking();
}

void Enemy::UpdateTracking(float deltaTime, Player* player) {
    UpdateFacingVec();
    MoveToPlayer(deltaTime, player);

    constexpr float attackStartRangeMargin = 1.5f;
    const float attackStartRange = mRadius + attackStartRangeMargin;
    if (IsPlayerInRange(attackStartRange, player))
        StartPreparingAttack();
}

void Enemy::UpdatePreparingAttack(float deltaTime) {
    UpdateFacingVec();
    mStandByAttackTimer -= deltaTime;
    if (mStandByAttackTimer <= 0.0f) {
        StartAttacking();
        return;
    }
    
    if (mIsJustBeforeAttack) return;

    if (IsJustBeforeAttack()) {
        mIsJustBeforeAttack = true;
        mGame->GetAudioSystem()->PlaySE("attackPreSE");
    }
}

void Enemy::UpdateAttacking(float deltaTime, Player* player) {
    MoveDuringAttacking(deltaTime, player);

    constexpr float hitRangeMargin = 0.2f;
    const float hitRange = mRadius + hitRangeMargin;
    if (IsPlayerInRange(hitRange, player) && !mIsHit && player->GetInvincibleTimer() <= 0.0f) {
        player->ApplyDamage(mAttack, mPos);
        mIsHit = true;
    }

    mAttackMotionTimer -= deltaTime;
    if (mAttackMotionTimer <= 0.0f)
        StartIdle();
}

void Enemy::UpdateKnockedBack(float deltaTime, Player* player) {
    MoveDuringKnockBack(deltaTime, player);

    mKnockBackTimer -= deltaTime;
    if (mKnockBackTimer <= 0.0f)
        StartIdle();
}

void Enemy::StartIdle() {
    mActionState = ActionState::Idle;
}

void Enemy::StartTracking() {
    mActionState = ActionState::Tracking;
}

void Enemy::StartPreparingAttack() {
    mActionState = ActionState::PreparingAttack;
    mStandByAttackTimer = mDefaultStandByAttackTimer;
}

void Enemy::StartAttacking() {
    mActionState = ActionState::Attacking;
    mAttackMotionTimer = mDefaultAttackMotionTimer;
    mIsHit = false;
    mIsJustBeforeAttack = false;
}

void Enemy::StartKnockedBack(float knockBackTimer, Player* player) {
    mActionState = ActionState::KnockedBack;
    mKnockBackTimer = knockBackTimer;
    mKnockBackFrom = glm::normalize(mPos - player->GetPos());
}

void Enemy::StartDying(Player* player) {
    mLifeState = LifeState::Dying;
    mDyingTimer = 1.0f;
    mHp = 0;
    constexpr float dyingKnockBackTimer = 1.0f;
    StartKnockedBack(dyingKnockBackTimer, player);
    mGame->GetAudioSystem()->PlaySE("defeatSE");
}

void Enemy::FinishDying() {
    mLifeState = LifeState::Dead;
    mIsActive = false;
    
    if (mIsBoss) {
        Star* star = GetCurrentPlanet()->GetStar();
        if (!star) return;

        star->SetIsActive(true); 
        star->SetPos(mPos);
    }
}

void Enemy::FinishLaunched() {
    mBreakCount = mBreakCountMax;
    mIsJudgeLanding = true;
    mLaunchedTimer = -1.0f;
}

bool Enemy::IsPlayerInRange(float range, Player* player) {
    glm::vec3 playerPos = player->GetPos();
    const float distToPlayer = glm::length(playerPos - mPos);

    return distToPlayer <= range;
}

bool Enemy::IsJustBeforeAttack() {
    constexpr float justBeforeAttackTime = 0.5f;
    return mStandByAttackTimer <= justBeforeAttackTime;
}

void Enemy::MoveToPlayer(float deltaTime, Player* player) {
    mPos += mFacingForwardVec * mMoveSpeed * deltaTime;
}

void Enemy::MoveDuringAttacking(float deltaTime, Player* player) {
    const float wrapTime = mDefaultAttackMotionTimer / 2;
    if (mAttackMotionTimer >= wrapTime) 
        mPos += mFacingForwardVec * mAttackSpeed * deltaTime;
    else 
        mPos -= mFacingForwardVec * mAttackSpeed * deltaTime;
}

void Enemy::MoveDuringKnockBack(float deltaTime, Player* player) {
    mPos += mKnockBackFrom * mKnockBackSpeed * deltaTime;
}

void Enemy::UpdateInAir(float deltaTime) {
    if (mLaunchedTimer >= 0.0f) {
        mLaunchedTimer -= deltaTime;

        if (mLaunchedTimer >= 0.0f) return;

        FinishLaunched();
        return;
    }

    glm::vec3 prevVelocity = mVelocity;
    ApplyGravity(deltaTime);
    
    const float vPrev = dot(prevVelocity, mUpVec);
    const float vNow  = dot(mVelocity, mUpVec);

    // 頂点で固定開始
    if (vPrev > 0.0f && vNow <= 0.0f)
        mLaunchedTimer = mDefaultLaunchedTimer;
}

void Enemy::ApplyCounter(Player* player) {
    constexpr float knockBackTimer = 0.6f;
    StartKnockedBack(knockBackTimer, player);

    mIsCountered = false;
    mHp -= player->GetAttack() * 2.0f;
    mStandByAttackTimer = -1.0f;

    if (mHp <= 0.0f)
        StartDying(player);
}

void Enemy::LaunchIntoAir(float deltaTime) {
    mGame->OnEnemyLaunched();

    constexpr float launchSpeed = 5.0f;
    mVelocity += mUpVec * launchSpeed;
    mPos += mVelocity * deltaTime;
    mOnGround = false;
    mStandByAttackTimer = -1.0f;
    mAttackMotionTimer = -1.0f;
    mIsJudgeLanding = false;
    mGame->GetAudioSystem()->PlaySE("breakSE");
    mGame->SetHitStopTimer(0.3f);

    StartIdle();
}

void Enemy::ApplyDamage(float damage, Player* player) {
    if (mLifeState != LifeState::Alive) return;

    mHp -= damage;

    if (mHp <= 0.0f)
        StartDying(player);

    if (mIsStrongAttacked) {
        constexpr float knockBackTimer = 1.0f;
        StartKnockedBack(knockBackTimer, player);

        mIsStrongAttacked = false;
        FinishLaunched();
        mGame->OnStrongAttacked();
    }
}

void Enemy::ApplyBreak(float deltaTime) {
    if (mLifeState != LifeState::Alive) return;

    mBreakCount--;

    if (mBreakCount <= 0) {
        LaunchIntoAir(deltaTime);
        return;
    }

    mGame->GetAudioSystem()->PlaySE("destroySE");
}