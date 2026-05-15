#include "Player.h"
#include "actor/Enemy.h"
#include "actor/Boat.h"
#include "actor/Key.h"
#include "Stage.h"
#include "actor/Star.h"
#include "Game.h"
#include "system/PhysicsSystem.h"
#include "state/UIState.h"
#include "actor/NPC.h"
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
    , mFacingYaw(0.0f)
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
    , mAttackIndex(0)
    , mAttackPressTimer(-1.0f)
    , mStrongAttackTimer(-1.0f)
    , mInvincibleTimer(-1.0f)
    , mComboTimer(-1.0f)
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

void Player::ProcessActor()
{   
    bool isPlaying = GetGame()->GetGameProgressState()->GetSceneState() == GameProgressState::SceneState::Playing;
    if(isPlaying) {
        ProcessGameController();
        ProcessKeyboard();
    }
}

void Player::ProcessGameController() {
    SDL_GameController* sdlController = GetGame()->GetSdlController();

    if (!sdlController || !SDL_GameControllerGetAttached(sdlController) || mPlayerNum != 1) return;
    
    const float deadZone = 0.25f;
    const float scale = 1.0f / 32767.0f; // SDL_GameControllerGetAxisの範囲が32767までで、scaleをかけて1.0f以内に抑えるため
    // 左スティックの操作をプレイヤー移動量に
    mMoveForward = SDL_GameControllerGetAxis(sdlController, SDL_CONTROLLER_AXIS_LEFTY) * scale;
    mMoveLeft = SDL_GameControllerGetAxis(sdlController, SDL_CONTROLLER_AXIS_LEFTX) * scale;

    // 触っていない時に動くのを防ぐ
    if (std::abs(mMoveForward) < deadZone)
        mMoveForward = 0.0f;
    if (std::abs(mMoveLeft) < deadZone)
        mMoveLeft = 0.0f;

    mJumpPressed = false;
    if (SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_A))
        mJumpPressed = true;

    mAttackPressed = false;
    if (SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_X))
        mAttackPressed = true;
    
    mWideAttackPressed = false;
    if (SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_Y))
        mWideAttackPressed = true;

    mDodgePressed = false;
    if (SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_B))
        mDodgePressed = true;

    mSpecialAttackPressed = false;
    if (SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_LEFTSHOULDER))
        mSpecialAttackPressed = true;
}

void Player::ProcessKeyboard() {
    if (!GetGame()->GetIsPlayer2Joined() || mPlayerNum == 1) return;
    
    GLFWwindow* window = GetGame()->GetWindow();
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
    // if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    //     mCameraYaw -= cameraSensitivity * deltaTime;
    // if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    //     mCameraYaw += cameraSensitivity * deltaTime;

    // if (std::abs(mMoveForward) > 0.01f || std::abs(mMoveLeft) > 0.01f)
    // {
    //     glm::vec3 moveDir = mForwardVec * mMoveForward + mLeftVec * mMoveLeft;
    //     float len = glm::length(moveDir);
    //     if (len > 0.001f)
    //     {
    //         moveDir /= len;
    //         mFacingYaw = GetYawFromDirection(up, moveDir) + 3.14159265f;
    //     }
    // }
    // GetPos() += mForwardVec * mMoveForward * dashSpeed * deltaTime;
    // GetPos() += mLeftVec * mMoveLeft * dashSpeed * deltaTime;
}

void Player::UpdateActor(float deltaTime)
{
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
}

void Player::Die() {
    Respawn();
}

void Player::Respawn() {
    mHp = mMaxHp;
    mPos = mRestartPos;
    mOnGround = true;
}

void Player::UpdateWorldVec() {
    glm::vec3 projectedForward = mForwardVec - glm::dot(mForwardVec, mUpVec) * mUpVec;

    if (glm::length(projectedForward) < 1e-6f) {
        projectedForward = glm::cross(glm::vec3(1, 0, 0), mUpVec);
        if (glm::length(projectedForward) < 1e-6f)
            projectedForward = glm::cross(glm::vec3(0, 1, 0), mUpVec);
    }
    projectedForward = glm::normalize(projectedForward);

    glm::vec3 worldLeft = glm::normalize(glm::cross(mUpVec, projectedForward));

    if (glm::length(mLeftVec) > 1e-6f && glm::dot(worldLeft, mLeftVec) < 0.0f)
        worldLeft = -worldLeft;

    mForwardVec = glm::normalize(projectedForward * std::cos(mCameraYaw)
                               - worldLeft * std::sin(mCameraYaw));

    mLeftVec = glm::normalize(glm::cross(mUpVec, mForwardVec));
}

void Player::UpdateIdle(float deltaTime) {
    if (!mOnGround) {
        ApplyGravity(deltaTime);

        if (mAttackPressed) {
            StartCharging(deltaTime);
            return;
        }
    }

    bool canDodge = mDodgeCooldown <= 0.0f && mAttackDodgeLockRemaining <= 0.0f && mCanDodge;
    if (canDodge && mDodgePressed)
        StartDodging();

    bool canAttack = mAttackCooldownRemaining <= 0.0f && mOnGround;
    if (canAttack && (mAttackPressed || mWideAttackPressed || mStrongAttackTimer >= 0.0f))
        StartAttacking(deltaTime);

    if ((std::abs(mMoveForward) > 0.01f || std::abs(mMoveLeft) > 0.01f))
        ChangeFaceDir();

    if (CanWalk())
        UpdateWalk(deltaTime);

    if (mJumpPressed && mOnGround)
        StartJumping(deltaTime);

    bool isRising = glm::dot(mVelocity, mUpVec) > 0.2f;
    if (!isRising) 
        mIsJudgeLanding = true;
    else 
        mIsJudgeLanding = false;

    if (mSpecialAttackPressed && mSpecialAttackCooldownRemaining <= 0.0f)
        SpecialAttack(deltaTime);
    
    UpdateBoatRide();
    mPos += mVelocity * deltaTime;
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
        StartStrongAttacking();
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
    if (mStrongAttackTimer <= 0.0f)
        StartIdle();
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

    if (mComboTimer > 0.0f) {
        mComboTimer -= deltaTime;
        if (mComboTimer <= 0.0f)
            mAttackIndex = 0; // コンボリセット
    }

    if (mAttackDodgeLockRemaining > 0.0f)
        mAttackDodgeLockRemaining -= deltaTime;

    if (mInvincibleTimer >= 0.0f)
        mInvincibleTimer -= deltaTime;

    if (mRayCastTimer >= 0.0f)
        mRayCastTimer -= deltaTime;
}

void Player::UpdateWalk(float deltaTime) {
    // プレイヤー移動（ダメージ時、空中固定時、回避時以外）
    glm::vec3 moveDelta = mForwardVec * mMoveForward * mMoveSpeed * deltaTime + mLeftVec * mMoveLeft * mMoveSpeed * deltaTime;
    glm::vec3 desiredPos = mPos + moveDelta;

    desiredPos = GetGame()->GetPhysicsSystem()->CheckCollision(moveDelta, desiredPos);

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
    GetGame()->GetAudioSystem()->PlaySE("dodgeSE");
}

void Player::StartAttacking(float deltaTime) {
    mActionState = ActionState::Attacking;
    
    if (mAttackPressed) {
        mAttackKind = AttackKind::Normal;
        mAttackRange = mNormalAttackRange;
        mAttackAngle = mNormalAttackAngle;
        mAttack = mNormalAttack;
    }
    else if (mWideAttackPressed) {
        mAttackKind = AttackKind::Wide;
        mAttackRange = mWideAttackRange;
        mAttackAngle = mWideAttackAngle;
        mAttack = mWideAttack;
    }
    else if (mStrongAttackTimer >= 0.0f) {
        mAttackKind = AttackKind::Strong;
        mAttackRange = mStrongAttackRange;
        mAttack = mStrongAttack;
    }

    Attack(deltaTime);
}

void Player::StartCharging(float deltaTime) {
    mActionState = ActionState::Charging;
    mAttackPressTimer = mDefaultAttackPressTimer;
    GetGame()->GetAudioSystem()->PlaySE("chargingSE");
}

void Player::StartStrongAttacking() {
    mActionState = ActionState::StrongAttacking;

    float pressTime = mDefaultAttackPressTimer - mAttackPressTimer / mDefaultAttackPressTimer;
    mStrongAttackTimer = mDefaultStrongAttackTimer * pressTime;
}

void Player::StartJumping(float deltaTime) {
    constexpr float jumpPower = 4.0f;
    mVelocity += mUpVec * jumpPower;
    mPos += mVelocity * deltaTime;
    mOnGround = false;
    GetGame()->GetAudioSystem()->PlaySE("jumpSE");
}

void Player::FinishCharging() {
    GetGame()->GetAudioSystem()->PlaySE("chargedSE");
}

void Player::MoveDuringDodging(float deltaTime) {
    const float dodgeSpeed = mDodgeDistance / mDodgeDuration;
    const glm::vec3 moveDelta = mDodgeDir * dodgeSpeed * deltaTime;
    glm::vec3 desiredPos = mPos + moveDelta;
    
    desiredPos = GetGame()->GetPhysicsSystem()->CheckCollision(moveDelta, desiredPos);
    mPos = desiredPos;

    auto sphereShape = Planet::PlanetShape::Sphere;
    if (mCurrentPlanet->GetPlanetShape() == sphereShape)
        FixPlanetSurface();
}

void Player::MoveDuringCharging(float deltaTime) {
    mPos += mUpVec * mChargeMoveSpeed * deltaTime;
}

void Player::MoveDuringStrongAttacking(float deltaTime) {
    mPos += -mUpVec * mStrongAttackSpeed * deltaTime;
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

void Player::ApplyGravity(float deltaTime) {
    float gravity = 9.8f;
    mVelocity -= mUpVec * gravity * deltaTime;
    mPos += mVelocity * deltaTime;

    if (GetGame()->GetCurrentStageNum() == 0 && mPos.y <= -1.0f) {
        mPos = mRestartPos;
        mOnGround = true;
    }

    if (mCurrentPlanet->GetPlanetShape() != Planet::PlanetShape::Sphere) return;
    
    glm::vec3 planetCenter = mCurrentPlanet->GetPos();
    float radius = mCurrentPlanet->GetRadius();
    float dist = glm::length(mPos - planetCenter);
    if (dist < radius * 0.5f)
    {
        mPos = mRestartPos;
        mCurrentPlanetNum = mRestartPlanetIndex;
        mCurrentPlanet = GetGame()->GetCurrentStage()->GetPlanets()[mCurrentPlanetNum];
        mVelocity = glm::vec3(0.0f);
        mOnGround = true;
    }
}

void Player::ChangeFaceDir() {
    glm::vec3 moveDir = glm::normalize(mForwardVec * mMoveForward + mLeftVec * mMoveLeft);
    mFacingForwardVec = moveDir;
    mFacingYaw = mGame->GetMathUtils()->GetYawFromDirection(mUpVec, moveDir) + 3.14159265f;
}

void Player::Attack(float deltaTime) {
    auto applyAttackLocksFromCooldown = [this]() {
        mAttackMoveLockRemaining = 0.2f;
        mComboTimer = mAttackMoveLockRemaining + 1.0f;
        if (mOnGround)
            mAttackMotionTimer = mDefaultAttackMotionTimer;
    };

    std::vector<Enemy*> hitEnemies = FindHitEnemies();
    if (hitEnemies.empty()) {
        
        return;
    }
        
    for (Enemy* enemy : hitEnemies)
            enemy->ApplyDamage(mAttack);
        
        mAttackCooldownRemaining = mLastAttackCooldown;
        mAttackIndex++;
        if (mAttackIndex == 3) {
            mAttackIndex = 0;
            for (Enemy* enemy : hitEnemies) {
                if (enemy->GetOnGround()) 
                    enemy->ApplyBreak(deltaTime);
            }
            applyAttackLocksFromCooldown();
        } else {
            applyAttackLocksFromCooldown();
            GetGame()->GetAudioSystem()->PlaySE("attackSE");
        }
}

std::vector<Enemy*> Player::FindHitEnemies() {
    std::vector<Enemy*> hitEnemies;

    for (Enemy* enemy : mCurrentPlanet->GetEnemies())
    {
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
    switch (mAttackKind) {
        case AttackKind::Normal:
            break;

        case AttackKind::Strong:
            return dist <= effectiveRange;

        case AttackKind::Wide:
            return dist <= effectiveRange && dot >= mAttackAngle;
    }

    return false;
}

// void Player::HandleMissedAttack() {
//     mAttackCooldownRemaining = mLastAttackCooldown;

//     applyAttackLocksFromCooldown();
//     GetGame()->GetAudioSystem()->PlaySE("attackMissSE");
// }

// void Player::HandleHitAttack(hitEnemies, deltaTime) {

// }

void Player::SpecialAttack(float deltaTime) {
    std::vector<Enemy*> enemies = mCurrentPlanet->GetEnemies();
    for (auto& enemy : enemies)
    {
        if (enemy->GetIsDead())
            continue;
        mAttack = mNormalAttack;
        enemy->ApplyDamage(mAttack);
        if (enemy->GetOnGround()) {
            enemy->ApplyBreak(deltaTime);
        }
        mSpecialAttackCooldownRemaining = mSpecialAttackCooldown;
    }
}

void Player::ApplyDamage(float damage, glm::vec3 knockBackFrom) {
    mHp -= damage;
    mKnockBackFrom = knockBackFrom;
    mDamageTimer = mDefaultDamageTimer;
    mInvincibleTimer = mDefaultInvincibleTimer;
    mActionState = ActionState::KnockedBack;
    GetGame()->GetAudioSystem()->PlaySE("damagedSE");
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

    std::vector<Planet*> planets = GetGame()->GetCurrentStage()->GetPlanets();
    mCurrentPlanet = planets[mCurrentPlanetNum];

    mPos = boat->GetDestPos();

    mRestartPos = mPos;
    mRestartPlanetIndex = mCurrentPlanetNum;

    mVelocity = glm::vec3(0.0f);

    mOnGround = true;
    mIsActive = true;

    UpdateFallbackUpVec();
}