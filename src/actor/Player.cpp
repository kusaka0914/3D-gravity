#include "Player.h"
#include "actor/Enemy.h"
#include "actor/Boat.h"
#include "Game.h"
#include "system/AudioSystem.h"
#include "system/PhysicsSystem.h"
#include "actor/Planet.h"
#include "utils/MathUtils.h"
#include "state/GameProgressState.h"
#include <btBulletDynamicsCommon.h>
#include <cmath>


Player::Player(Game* game)
    : CharacterActor(game)
    , mKnockBackFrom(0.0f)
    , mRestartPos(0.0f)
    , mCurrentPlanetNum(0)
    , mCameraYaw(0.0f)
    , mCameraPitch(0.4f)
    , mMoveForward(0.0f)
    , mMoveLeft(0.0f)
    , mAttackStartHeight(0.0f)
    , mDodgeTimer(0.0f)
    , mDodgeCooldown(0.0f)
    , mMoveSpeed(10.2f)
    , mCameraStickX(0.0f)
    , mCameraStickY(0.0f)
    , mAttack(10.0f)
    , mHp(100.0f)
    , mDodgePressed(false)
    , mJumpPressed(false)
    , mAttackPressed(false)
    , mSpecialAttackPressed(false)
    , mDamageTimer(0.0f)
    , mAttackCooldownRemaining(0.0f)
    , mAttackMoveLockRemaining(0.0f)
    , mAttackDodgeLockRemaining(0.0f)
    , mAttackComboIndex(0)
    , mAttackPressTimer(-1.0f)
    , mStrongAttackTimer(-1.0f)
    , mInvincibleTimer(-1.0f)
    , mComboKeepTimer(-1.0f)
    , mSpecialAttackCooldownRemaining(-1.0f)
    , mRayCastTimer(0.5f)
    , mCanDodge(true)
    , mActionState(ActionState::Idle)
{

}

Player::~Player()
{
    
}

void Player::Initialize() {
    mRestartPlanetIndex = mCurrentPlanetNum;
    mRestartPos = mPos;
}

void Player::ProcessActor() {   
    ProcessGameController();
    ProcessKeyboard();
}

void Player::ProcessGameController() {
    SDL_GameController* sdlController = mGame->GetSdlController();

    if (!sdlController || !SDL_GameControllerGetAttached(sdlController) || mPlayerNum != 1) return;
    
    constexpr float deadZone = 0.25f;
    constexpr float scale = 1.0f / 32767.0f; // SDL_GameControllerGetAxisの範囲が32767までで、scaleをかけて1.0f以内に抑えるため

    mMoveForward = SDL_GameControllerGetAxis(sdlController, SDL_CONTROLLER_AXIS_LEFTY) * scale;
    mMoveLeft = SDL_GameControllerGetAxis(sdlController, SDL_CONTROLLER_AXIS_LEFTX) * scale;

    if (std::abs(mMoveForward) < deadZone)
        mMoveForward = 0.0f;
    if (std::abs(mMoveLeft) < deadZone)
        mMoveLeft = 0.0f;

    mJumpPressed = SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_A);
    mAttackPressed = SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_X);
    mWideAttackPressed = SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_Y);
    mDodgePressed = SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_B);
    mSpecialAttackPressed = SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
}

void Player::ProcessKeyboard() {
    if (!mGame->GetIsPlayer2Joined() || mPlayerNum == 1) return;
    
    GLFWwindow* window = mGame->GetWindow();
    mJumpPressed = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
    mMoveSpeed = (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) ? 2.0f : 3.0f;
    mMoveForward = 0.0f;
    mMoveLeft = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        mMoveForward -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        mMoveForward += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        mMoveLeft -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        mMoveLeft += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        mSpecialAttackPressed = true;
}

void Player::UpdateActor(float deltaTime) {
    CharacterActor::UpdateActor(deltaTime);

    bool isPlaying = mGame->GetGameProgressState()->GetSceneState() == GameProgressState::SceneState::Playing;
    if(!isPlaying) return;
    
    if (IsAlive())
        UpdateAlive(deltaTime);
    else 
        Die();
}

void Player::UpdateAlive(float deltaTime) {
    UpdateWorldVec();

    switch (mActionState)
    {
        case ActionState::Idle:
            UpdateIdle(deltaTime);
            break;

        case ActionState::Dodging:
            UpdateDodging(deltaTime);
            break;

        case ActionState::Attacking:
            UpdateAttacking(deltaTime);
            break;

        case ActionState::Charging:
            UpdateCharging(deltaTime);
            break;

        case ActionState::StrongAttacking:
            UpdateStrongAttacking(deltaTime);
            break;
        
        case ActionState::KnockedBack:
            UpdateKnockedBack(deltaTime);
            break;
    }

    UpdateTimer(deltaTime);

    mDodgePressedPrev = mDodgePressed;
    mAttackPressedPrev = mAttackPressed;
    mWideAttackPressedPrev = mWideAttackPressed;
}

void Player::Die() {
    Recover();
    Respawn();
}

void Player::Recover() {
    mHp = mMaxHp;
}

void Player::Respawn() {
    mPos = mRestartPos;
    mOnGround = true;
}

void Player::UpdateWorldVec() {
    // mForwardVecをmUpVecに垂直な平面へ投影して、地面に沿った前方向を作る
    glm::vec3 projectedForward = mForwardVec - glm::dot(mForwardVec, mUpVec) * mUpVec;

    if (glm::length(projectedForward) < 1e-6f) {
        projectedForward = glm::cross(glm::vec3(1, 0, 0), mUpVec);
        if (glm::length(projectedForward) < 1e-6f)
            projectedForward = glm::cross(glm::vec3(0, 1, 0), mUpVec);
    }

    projectedForward = glm::normalize(projectedForward);
    glm::vec3 worldLeft = glm::normalize(glm::cross(mUpVec, projectedForward));

    // 地面に沿った前方向を、mCameraYaw分だけmUpVec軸まわりに回転させる
    mForwardVec = glm::normalize(projectedForward * std::cos(mCameraYaw) - worldLeft * std::sin(mCameraYaw));
    mLeftVec = glm::normalize(glm::cross(mUpVec, mForwardVec));
}

void Player::UpdateIdle(float deltaTime) {
    UpdateBoatRide();

    if (!mIsActive) return;

    bool canStartCharging = !mOnGround && mAttackPressed;
    if (canStartCharging) {
        StartCharging(deltaTime);
        return;
    }

    ApplyGravity(deltaTime);

    bool canStartDodging = mDodgeCooldown <= 0.0f && mAttackDodgeLockRemaining <= 0.0f && mCanDodge && mDodgePressed && !mDodgePressedPrev;
    if (canStartDodging) {
        StartDodging();
        return;
    }

    bool canStartAttacking = mAttackCooldownRemaining <= 0.0f && mOnGround && ((mAttackPressed || mWideAttackPressed) && !mAttackPressedPrev && !mWideAttackPressedPrev);
    if (canStartAttacking) {
        StartAttacking(deltaTime);
        return;
    }

    bool canStartJumping = mJumpPressed && mOnGround;
    if (canStartJumping) {
        StartJumping(deltaTime);
        return;
    }

    bool canSpecialAttack = mSpecialAttackPressed && mSpecialAttackCooldownRemaining <= 0.0f;
    if (canSpecialAttack) {
        SpecialAttack(deltaTime);
        return;
    }

    bool isMoving = std::abs(mMoveForward) > 0.01f || std::abs(mMoveLeft) > 0.01f;
    if (isMoving)
        ChangeFaceDir();

    if (CanWalk())
        UpdateWalk(deltaTime);

    if (IsFallIntoPlanetInside())
        Respawn();
    
    bool isFalling = glm::dot(mVelocity, mUpVec) < 0.0f;
    if (isFalling)
        mIsJudgeLanding = true;
}

bool Player::IsFallIntoPlanetInside() {
    if (mCurrentPlanet->GetPlanetShape() != Planet::PlanetShape::Sphere) return false;

    float dist = glm::length(mPos - mCurrentPlanet->GetPos());
    const float planetHalfRadius = mCurrentPlanet->GetRadius() * 0.5f;

    if (dist < planetHalfRadius)
        return true;

    return false;
}

void Player::UpdateDodging(float deltaTime) {
    MoveDuringDodging(deltaTime);

    mDodgeTimer -= deltaTime;
    if (mDodgeTimer <= 0.0f)
        StartIdle();
}

void Player::UpdateAttacking(float deltaTime) {
    MoveDuringAttacking(deltaTime);

    if (CanWalk())
        UpdateWalk(deltaTime);
    
    mAttackMotionTimer -= deltaTime;
    if (mAttackMotionTimer <= 0.0f)
        StartIdle();
}

void Player::UpdateCharging(float deltaTime) {
    bool isAttackBtnReleased = !mAttackPressed;
    if (isAttackBtnReleased) {
        StartStrongAttacking(deltaTime);
        return;
    }
    
    if (mAttackPressTimer < 0.0f) return;

    mAttackPressTimer -= deltaTime;
    if (mAttackPressTimer >= 0.0f) {
        MoveDuringCharging(deltaTime);
        return;
    }

    FinishCharging();
}

void Player::UpdateStrongAttacking(float deltaTime) {
    MoveDuringStrongAttacking(deltaTime);

    mStrongAttackTimer -= deltaTime;
    if (mStrongAttackTimer >= 0.0f) return;

    StartIdle();

    if (mIsStrongAttackHit){
        mGame->SetHitStopTimer(0.3f);
        mIsStrongAttackHit = false;
    }
}

void Player::UpdateKnockedBack(float deltaTime) {
    MoveDuringKnockBack(deltaTime);
    
    mDamageTimer -= deltaTime;
    if (mDamageTimer <= 0.0f)
        StartIdle();
}

void Player::UpdateTimer(float deltaTime) {
    if (mDodgeCooldown > 0.0f)
        mDodgeCooldown -= deltaTime;

    if (mSpecialAttackCooldownRemaining >= 0.0f)
        mSpecialAttackCooldownRemaining -= deltaTime;

    if (mAttackCooldownRemaining >= 0.0f)
        mAttackCooldownRemaining -= deltaTime;

    if (mAttackMoveLockRemaining > 0.0f)
        mAttackMoveLockRemaining -= deltaTime;

    if (mAttackDodgeLockRemaining > 0.0f)
        mAttackDodgeLockRemaining -= deltaTime;

    if (mInvincibleTimer >= 0.0f)
        mInvincibleTimer -= deltaTime;

    if (mRayCastTimer >= 0.0f)
        mRayCastTimer -= deltaTime;

    if (mComboKeepTimer > 0.0f) {
        mComboKeepTimer -= deltaTime;
        if (mComboKeepTimer >= 0.0f) return;
        
        mAttackComboIndex = 0;
    }
}

void Player::UpdateWalk(float deltaTime) {
    glm::vec3 moveDelta = mForwardVec * mMoveForward * mMoveSpeed * deltaTime + mLeftVec * mMoveLeft * mMoveSpeed * deltaTime;
    glm::vec3 desiredPos = mPos + moveDelta;

    desiredPos = mGame->GetPhysicsSystem()->CheckCollision(this, moveDelta, desiredPos);
    mPos = desiredPos;
}

void Player::UpdateBoatRide() {
    const std::vector<Boat*>& boats = mCurrentPlanet->GetBoats();
    if (boats.empty()) return;

    for (auto boat : boats) {
        if (!boat->GetIsActive()) continue;

        if (boat->GetIsMoving()) {
            FollowMovingBoat(boat);
            return;
        }
        
        if (IsTouchingBoat(boat)) {
            StartRidingBoat(boat);
            return;
        }
    }
}

void Player::StartIdle() {
    mActionState = ActionState::Idle;
}

void Player::StartDodging() {
    mActionState = ActionState::Dodging;

    if (mMoveForward != 0.0f || mMoveLeft != 0.0f)
        mDodgeDir = mFacingForwardVec;
    else
        mDodgeDir = -mFacingForwardVec;

    mDodgeTimer = mDodgeDuration;
    mDodgeCooldown = mDodgeCooldownTime;
    
    mDodgeStartHeight = glm::length(mPos - mCurrentPlanet->GetPos());
    mVelocity = glm::vec3(0.0f);
    mGame->GetAudioSystem()->PlaySE("dodgeSE");
}

void Player::StartAttacking(float deltaTime) {
    mActionState = ActionState::Attacking;
    
    if (mAttackPressed) {
        mAttackKind = AttackKind::Normal;
        mAttackRange = mNormalAttackRange;
        mAttackAngle = mNormalAttackAngle;
        mAttackCooldownRemaining = mLastAttackCooldown;
        mAttack = mNormalAttack;
    }
    else if (mWideAttackPressed) {
        mAttackKind = AttackKind::Wide;
        mAttackRange = mWideAttackRange;
        mAttackAngle = mWideAttackAngle;
        mAttackCooldownRemaining = mAttackCooldown;
        mAttack = mWideAttack;
    }

    Attack(deltaTime);
}

void Player::StartCharging(float deltaTime) {
    mActionState = ActionState::Charging;
    mAttackPressTimer = mDefaultAttackPressTimer;
    mGame->GetAudioSystem()->PlaySE("chargingSE");
}

void Player::StartStrongAttacking(float deltaTime) {
    mActionState = ActionState::StrongAttacking;
    mAttackKind = AttackKind::Strong;
    mAttackRange = mStrongAttackRange;
    mAttackCooldownRemaining = mLastAttackCooldown;
    mAttack = mStrongAttack;

    float pressTime = std::min(1.0f, mDefaultAttackPressTimer - mAttackPressTimer / mDefaultAttackPressTimer);
    mStrongAttackTimer = mDefaultStrongAttackTimer * pressTime;
    Attack(deltaTime);
}

void Player::StartJumping(float deltaTime) {
    constexpr float jumpPower = 6.0f;
    mVelocity += mUpVec * jumpPower;
    mPos += mVelocity * deltaTime;
    mOnGround = false;
    mIsJudgeLanding = false;
    mGame->GetAudioSystem()->PlaySE("jumpSE");
}

void Player::FinishCharging() {
    mGame->GetAudioSystem()->PlaySE("chargedSE");
}

void Player::MoveDuringDodging(float deltaTime) {
    const float dodgeSpeed = mDodgeDistance / mDodgeDuration;
    const glm::vec3 moveDelta = mDodgeDir * dodgeSpeed * deltaTime;
    glm::vec3 desiredPos = mPos + moveDelta;
    
    desiredPos = mGame->GetPhysicsSystem()->CheckCollision(this, moveDelta, desiredPos);
    mPos = desiredPos;

    auto sphereShape = Planet::PlanetShape::Sphere;
    if (mCurrentPlanet->GetPlanetShape() == sphereShape)
        FixPlanetSurface();
}

void Player::MoveDuringCharging(float deltaTime) {
    mPos += -mFacingForwardVec * mChargeMoveSpeed * deltaTime;
}

void Player::MoveDuringStrongAttacking(float deltaTime) {
    mPos += mFacingForwardVec * mStrongAttackSpeed * deltaTime;
}

void Player::MoveDuringAttacking(float deltaTime) {
    const float wrapTime = mDefaultAttackMotionTimer / 2.0f;
    if (mAttackMotionTimer >= wrapTime)
        mPos += mFacingForwardVec * mAttackSpeed * deltaTime;
    else 
        mPos -= mFacingForwardVec * mAttackSpeed * deltaTime;
}

void Player::MoveDuringKnockBack(float deltaTime) {
    glm::vec3 toPlayer = glm::normalize(mPos - mKnockBackFrom);
    mPos += toPlayer * deltaTime;
}

void Player::FixPlanetSurface() {
    glm::vec3 planetCenter = mCurrentPlanet->GetPos();
    glm::vec3 planetSurface = planetCenter + glm::normalize(mPos - planetCenter) * mDodgeStartHeight;
    mPos = planetSurface;
}

void Player::ChangeFaceDir() {
    glm::vec3 moveDir = glm::normalize(mForwardVec * mMoveForward + mLeftVec * mMoveLeft);
    mFacingForwardVec = moveDir;
    mFacingYaw = mGame->GetMathUtils()->GetYawFromDirection(mUpVec, moveDir) + 3.14159265f;
}

void Player::Attack(float deltaTime) {
    std::vector<Enemy*> hitEnemies = FindHitEnemies();
    if (hitEnemies.empty()) {
        StartAfterAttackReaction();
        GetGame()->GetAudioSystem()->PlaySE("attackMissSE");

        if (mAttackComboIndex != 3) return;

        mAttackComboIndex = 0;
        return;
    }

    if (mAttackKind != AttackKind::Strong) {
        StartAfterAttackReaction();
        for (Enemy* enemy : hitEnemies)
                enemy->ApplyDamage(mAttack);

        if (mAttackComboIndex != 3) {
            mGame->GetAudioSystem()->PlaySE("attackSE");
            return;
        }

        mAttackComboIndex = 0;
        for (Enemy* enemy : hitEnemies) {
            if (enemy->GetOnGround()) 
                enemy->ApplyBreak(deltaTime);
        }
        return;
    }

    GetGame()->GetAudioSystem()->PlaySE("attackAirSE");
    for (Enemy* enemy : hitEnemies) {
        enemy->SetIsStrongAttacked(true);
        enemy->ApplyDamage(mAttack);
        mIsStrongAttackHit = true;
    }
}

void Player::StartAfterAttackReaction() {
    mAttackMoveLockRemaining = 0.2f;
    mComboKeepTimer = mAttackMoveLockRemaining + 1.0f;

    if (mOnGround)
        mAttackMotionTimer = mDefaultAttackMotionTimer;

    mAttackComboIndex++;
    if (mAttackComboIndex == 3) {
        mAttackCooldownRemaining = mLastAttackCooldown;
    }
}

std::vector<Enemy*> Player::FindHitEnemies() {
    std::vector<Enemy*> hitEnemies;

    for (Enemy* enemy : mCurrentPlanet->GetEnemies()) {
        if (enemy->GetIsDead())
            continue;

        if (mAttackKind == AttackKind::Strong && enemy->GetOnGround())
            continue;

        const glm::vec3 enemyPos = enemy->GetPos();
        const glm::vec3 toEnemy = glm::normalize(enemyPos - mPos);
        const float dist = glm::length(enemyPos - mPos);
        const float dot = glm::dot(mFacingForwardVec, toEnemy);
        const float effectiveRange = mAttackRange + enemy->GetRadius();

        if (IsEnemyHitByAttack(dist, dot, effectiveRange))
            hitEnemies.push_back(enemy);
    }

    return hitEnemies;
}

bool Player::IsEnemyHitByAttack(float dist, float dot, float effectiveRange) {
    if (mAttackKind == AttackKind::Wide)
        return dist <= effectiveRange && dot >= mAttackAngle;

    return dist <= effectiveRange;
}

void Player::SpecialAttack(float deltaTime) {
    std::vector<Enemy*> enemies = mCurrentPlanet->GetEnemies();
    for (auto& enemy : enemies) {
        if (enemy->GetIsDead())
            continue;

        mAttack = mNormalAttack;
        enemy->ApplyDamage(mAttack);
        
        if (enemy->GetOnGround()) 
            enemy->ApplyBreak(deltaTime);

        mSpecialAttackCooldownRemaining = mSpecialAttackCooldown;
    }
}

void Player::ApplyDamage(float damage, glm::vec3 knockBackFrom) {
    mHp -= damage;
    mKnockBackFrom = knockBackFrom;
    mDamageTimer = mDefaultDamageTimer;
    mInvincibleTimer = mDefaultInvincibleTimer;
    mActionState = ActionState::KnockedBack;
    mGame->GetAudioSystem()->PlaySE("damagedSE");
}

void Player::FollowMovingBoat(Boat* boat) {
    mPos = boat->GetPos();
}

bool Player::IsTouchingBoat(Boat* boat) {
    float distToBoat = glm::length(mPos - boat->GetPos());
    constexpr float boatTouchRadius = 0.9f;
    return distToBoat <= boatTouchRadius;
}

void Player::StartRidingBoat(Boat* boat) {
    boat->StartTravel();
    mIsActive = false;
}

void Player::OnBoatArrived(Boat* boat) {
    mCurrentPlanetNum++;

    mPos = boat->GetDestPos();
    mRestartPos = mPos;
    mRestartPlanetIndex = mCurrentPlanetNum;

    mVelocity = glm::vec3(0.0f);

    mOnGround = true;
    mIsActive = true;

    UpdateFallbackUpVec();
}