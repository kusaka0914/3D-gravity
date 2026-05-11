#include "Player.h"
#include "Enemy.h"
#include "Boat.h"
#include "Key.h"
#include "Stage.h"
#include "Star.h"
#include "Game.h"
#include "PhysicsSystem.h"
#include "UIState.h"
#include "NPC.h"
#include "Helper.h"
#include "GameProgressState.h"
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
    , mIsDamaged(false)
    , mIsDamagePrev(false)
    , mDodgePressed(false)
    , mDodgePressedPrev(false)
    , mJumpPressed(false)
    , mAttackPressed(false)
    , mAttackPressedPrev(false)
    , mSpecialAttackPressed(false)
    , mSpecialAttackPressedPrev(false)
    , mDamageTimer(0.0f)
    , mAttackCooldownRemaining(0.0f)
    , mAttackMoveLockRemaining(0.0f)
    , mAttackDodgeLockRemaining(0.0f)
    , mAttackIndex(0)
    , mIsActive(true)
    , mAttackPressTimer(-1.0f)
    , mStrongAttackTimer(-1.0f)
    , mInvincibleTimer(-1.0f)
    , mComboTimer(-1.0f)
    , mSpecialAttackCooldownRemaining(-1.0f)
    , mRayCastTimer(0.5f)
    , mCanDodge(true)
{
    
}

Player::~Player()
{
    
}

void Player::ProcessActor()
{   
    bool isPlaying = GetGame()->GetGameProgressState()->GetSceneState() == GameProgressState::SceneState::Playing;
    if(!isPlaying) {
        mCanMove = false;
    }
    
    ProcessGameController();
    ProcessKeyboard();
}

void Player::UpdateActor(float deltaTime)
{
    UpdateCamera(deltaTime);

    UpdateWorldVec();

    UpdateCameraSmoothing(deltaTime);

    if (mDodgeTimer <= 0.0f && mAttackPressTimer < 0.0f && (std::abs(mMoveForward) > 0.01f || std::abs(mMoveLeft) > 0.01f))
        ChangeFaceDir();

    if (!mIsDamaged && mAttackMoveLockRemaining <= 0.0f && mDodgeTimer <= 0.0f && mAttackPressTimer < 0.0f && mCanMove)
        UpdateWalk(deltaTime);

    UpdateDodge(deltaTime);

    if (mDodgeTimer <= 0.0f && mAttackPressTimer < 0.0f && mStrongAttackTimer <= 0.0f)
        ApplyGravity(deltaTime);
    
    bool isRising = glm::dot(mVelocity, mUpVec) > 0.2f;
    if (!isRising) 
        mIsJudgeLanding = true;
    else 
        mIsJudgeLanding = false;

    // 攻撃
    bool canAttack = ((mAttackPressed && !mAttackPressedPrev) || (mWideAttackPressed && !mWideAttackPressedPrev)) && mAttackCooldownRemaining <= 0.0f && mOnGround || mStrongAttackTimer >= 0.0f;
    if (canAttack)
    {
        Attack(deltaTime);
    }

    if (mSpecialAttackPressed && !mSpecialAttackPressedPrev && mSpecialAttackCooldownRemaining <= 0.0f)
        SpecialAttack();

    if (!mOnGround)
        ChargeAttack(deltaTime);

    if (mIsDamaged)
        TakeDamage();
    
    if (mHp <= 0)
        Die();

    std::vector<Boat*> boats = mCurrentPlanet->GetBoats();
    if (!boats.empty()) {
        RideBoat();
    }

    UpdateTimer(deltaTime);

    UpdatePrev();
}

void Player::ProcessGameController() {
    SDL_GameController* sdlController = GetGame()->GetSdlController();

    if (!sdlController || !SDL_GameControllerGetAttached(sdlController) || mPlayerNum != 1 || mDamageTimer > 0.0f || !mCanMove)
        return;
    
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

    // 右スティックの操作をカメラ移動量に
    mCameraStickY = SDL_GameControllerGetAxis(sdlController, SDL_CONTROLLER_AXIS_RIGHTY) * scale;
    mCameraStickX = SDL_GameControllerGetAxis(sdlController, SDL_CONTROLLER_AXIS_RIGHTX) * scale;

    if (std::abs(mCameraStickY) < deadZone)
        mCameraStickY = 0.0f;
    if (std::abs(mCameraStickX) < deadZone)
        mCameraStickX = 0.0f;

    // ジャンプ判定（Aボタン）
    mJumpPressed = false;
    if (SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_A))
        mJumpPressed = true;

    // 攻撃判定（Xボタン）
    mAttackPressed = false;
    if (SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_X))
        mAttackPressed = true;
    
    // 攻撃判定（Xボタン）
    mWideAttackPressed = false;
    if (SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_Y))
        mWideAttackPressed = true;

    // 回避（Bボタン）
    mDodgePressed = false;
    if (SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_B))
        mDodgePressed = true;

    // スペシャルアタック（Lボタン）
    mSpecialAttackPressed = false;
    if (SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_LEFTSHOULDER))
        mSpecialAttackPressed = true;
}

void Player::ProcessKeyboard() {
    if (!GetGame()->GetIsPlayer2Joined() || mPlayerNum == 1)
        return;
    
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

void Player::UpdateCamera(float deltaTime) {
    const float cameraSensitivity = 2.5f;

    mCameraYaw = mCameraStickX * cameraSensitivity * deltaTime;
    mCameraPitch -= mCameraStickY * cameraSensitivity * deltaTime;
    mCameraPitch = glm::clamp(mCameraPitch, -1.2f, -0.2f);
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

void Player::UpdateWalk(float deltaTime) {
    // プレイヤー移動（ダメージ時、空中固定時、回避時以外）
    glm::vec3 moveDelta = mForwardVec * mMoveForward * mMoveSpeed * deltaTime + mLeftVec * mMoveLeft * mMoveSpeed * deltaTime;
    glm::vec3 desiredPos = mPos + moveDelta;

    desiredPos = GetGame()->GetPhysicsSystem()->CheckCollision(moveDelta, desiredPos);

    mPos = desiredPos;
    // ジャンプ処理
    if (mOnGround && mJumpPressed)
    {
        float jumpVel = 5.0f;
        mVelocity += mUpVec * jumpVel;
        mOnGround = false;
        GetGame()->GetAudioSystem()->PlaySE("jumpSE");
    }
}

void Player::UpdateDodge(float deltaTime) {
    // 向いている方向へ回避開始
    if (mDodgePressed && !mDodgePressedPrev && mDodgeCooldown <= 0.0f && mAttackDodgeLockRemaining <= 0.0f && mCanDodge)
        StartDodge();
    
    // 回避中移動
    if (mDodgeTimer > 0.0f)
        Dodge(deltaTime);
}

void Player::StartDodge() {
    if (mMoveForward != 0.0f || mMoveLeft != 0.0f)
        mDodgeDir = mFacingForwardVec;
    else
        mDodgeDir = -mFacingForwardVec;
    mDodgeTimer = mDodgeDuration;
    if (mOnGround) 
        mDodgeCooldown = mDodgeCooldownTime;
    else
        mDodgeCooldown = mDodgeCooldownTime * 3;
    
    glm::vec3 center = mCurrentPlanet->GetPos();
    mDodgeStartHeight = glm::length(mPos - center);
    mVelocity = glm::vec3(0.0f);
    GetGame()->GetAudioSystem()->PlaySE("dodgeSE");
    mCanDodge = false;
}

void Player::Dodge(float deltaTime) {
    float dodgeSpeed = mDodgeDistance / mDodgeDuration;
    glm::vec3 moveDelta = mDodgeDir * dodgeSpeed * deltaTime;
    glm::vec3 desiredPos = mPos + moveDelta;
    
    desiredPos = GetGame()->GetPhysicsSystem()->CheckCollision(moveDelta, desiredPos);
    mPos = desiredPos;

    glm::vec3 center = mCurrentPlanet->GetPos();
    // 空中回避：直前の高さを維持して浮遊
    auto planetShape = mCurrentPlanet->GetPlanetShape();
    auto sphereShape = Planet::PlanetShape::Sphere;

    if (planetShape != sphereShape)
        return;

    mPos = center + glm::normalize(mPos - center) * mDodgeStartHeight;
}

void Player::UpdateCameraSmoothing(float deltaTime)
{
    float upSmooth = 1.0f - std::exp(-8.0f * deltaTime);
    mCameraUpVec = glm::normalize(glm::mix(mCameraUpVec, mUpVec, upSmooth));

    float targetSmooth = 1.0f - std::exp(-10.0f * deltaTime);
    mCameraTargetPos = glm::mix(mCameraTargetPos, mPos, targetSmooth);
}

void Player::ApplyGravity(float deltaTime) {
    float gravity = 9.8f;
    mVelocity -= mUpVec * gravity * deltaTime;
    mPos += mVelocity * deltaTime;

    if (GetGame()->GetCurrentStageNum() == 0 && mPos.y <= -1.0f) {
        mPos = mRestartPos;
        mCurrentPlanetNum = mRestartPlanetIndex;
        mCurrentPlanet = GetGame()->GetCurrentStage()->GetPlanets()[mCurrentPlanetNum];
        mVelocity = glm::vec3(0.0f);
        mOnGround = true;
    }

    if (mCurrentPlanet->GetPlanetShape() == Planet::PlanetShape::Normal)
        return;
    
    glm::vec3 center = mCurrentPlanet->GetPos();
    float radius = mCurrentPlanet->GetRadius();
    float dist = glm::length(mPos - center);
    // 落下して惑星内部にめり込んだらリスタート地点へ
    // if (dist < radius * 0.5f)
    // {
    //     mPos = mRestartPos;
    //     mCurrentPlanetNum = mRestartPlanetIndex;
    //     mCurrentPlanet = GetGame()->GetCurrentStage()->GetPlanets()[mCurrentPlanetNum];
    //     mVelocity = glm::vec3(0.0f);
    //     mOnGround = true;
    // }
}

void Player::ChangeFaceDir() {
    glm::vec3 moveDir = mForwardVec * mMoveForward + mLeftVec * mMoveLeft;
    float len = glm::length(moveDir);
    if (len > 0.001f) {
        moveDir /= len;
        mFacingForwardVec = moveDir;
        mFacingLeftVec = glm::normalize(glm::cross(mUpVec, mFacingForwardVec));

        // 描画用に yaw が必要なら残す
        mFacingYaw = mGame->GetHelper()->GetYawFromDirection(mUpVec, moveDir) + 3.14159265f;
    }
}

void Player::Attack(float deltaTime) {
    mAttackStartHeight = glm::length(mPos - mCurrentPlanet->GetPos());
    mVelocity = glm::vec3(0.0f);

    auto applyAttackLocksFromCooldown = [this]() {
        // mAttackMoveLockRemaining = std::min(mAttackCooldownRemaining, 0.5f) + 0.2f;
        mAttackMoveLockRemaining = 0.2f;
        mAttackDodgeLockRemaining = std::max(0.0f, mAttackMoveLockRemaining - 0.5f);
        mComboTimer = mAttackMoveLockRemaining + 1.0f;
        if (!mOnGround)
        {

        } else {
            mAttackMotionTimer = mDefaultAttackMotionTimer;;
        }
    };
    
    if (mAttackPressed) {
        mAttackRange = mNormalAttackRange;
        mAttackAngle = mNormalAttackAngle;
        mAttack = mNormalAttack;
    } else if (mWideAttackPressed) {
        mAttackRange = mWideAttackRange;
        mAttackAngle = mWideAttackAngle;
        mAttack = mWideAttack;
    } else if (mStrongAttackTimer >= 0.0f) {
        mAttackRange = mStrongAttackRange;
        mAttack = mStrongAttack;
    }

    std::vector<Enemy*> enemies = mCurrentPlanet->GetEnemies();
    std::vector<Enemy*> hitEnemies;
    for (auto& enemy : enemies)
    {
        if (!enemy->GetIsAlive())
            continue;
        if (mStrongAttackTimer >= 0.0f && enemy->GetOnGround())
            continue;
        glm::vec3 enemyPos = enemy->GetPos();
        glm::vec3 toEnemy = glm::normalize(enemyPos - mPos);
        float dist = glm::length(enemyPos - mPos);
        float dot = glm::dot(mFacingForwardVec, toEnemy);
        float effectiveRange = mAttackRange + enemy->GetRadius();
        if (mStrongAttackTimer >= 0.0f && dist <= effectiveRange)
            hitEnemies.push_back(enemy);

        if (mAttackPressed && dist <= effectiveRange)
            hitEnemies.push_back(enemy);

        if (mWideAttackPressed && dist <= effectiveRange && dot >= mAttackAngle)
            hitEnemies.push_back(enemy);
    }

    if (!hitEnemies.empty())
    {
        for (Enemy* enemy : hitEnemies)
            enemy->SetIsDamaged(true);

        if (mWideAttackPressed)
            mAttackCooldownRemaining = mAttackCooldown;
        else if (mAttackPressed)
            mAttackCooldownRemaining = mLastAttackCooldown;
        mAttackIndex++;
        float strongAttackTimerNext = mStrongAttackTimer - deltaTime;
        if (mIsStrongAttack) {
            for (Enemy* enemy : hitEnemies) {
                enemy->SetIsStrongAttacked(true);
                mAttackIndex = 0;
            }
            GetGame()->GetAudioSystem()->PlaySE("attackAirSE");
            mAttackIndex = 0;
            mIsStrongAttack = false;
        }
        if (mAttackIndex == 3)
        {
            mAttackCooldownRemaining = mLastAttackCooldown;
            mAttackIndex = 0;
            for (Enemy* enemy : hitEnemies)
            {
                if (enemy->GetOnGround()) {
                    enemy->SetIsBroken(true);
                }
            }
            applyAttackLocksFromCooldown();
        } else {
            applyAttackLocksFromCooldown();
            GetGame()->GetAudioSystem()->PlaySE("attackSE");
        }
    }
    else
    {
        if (mWideAttackPressed)
            mAttackCooldownRemaining = mAttackCooldown;
        else if (mAttackPressed)
            mAttackCooldownRemaining = mLastAttackCooldown;
        mAttackIndex++;
        if (mAttackIndex == 3)
        {
            mAttackCooldownRemaining = mLastAttackCooldown;
            mAttackIndex = 0;
        }
        applyAttackLocksFromCooldown();
        GetGame()->GetAudioSystem()->PlaySE("attackMissSE");
    }
}

void Player::SpecialAttack() {
    std::vector<Enemy*> enemies = mCurrentPlanet->GetEnemies();
    for (auto& enemy : enemies)
    {
        if (!enemy->GetIsAlive())
            continue;
        mAttack = mNormalAttack;
        enemy->SetIsDamaged(true);
        if (enemy->GetOnGround()) {
            enemy->SetIsBroken(true);
        }
        mSpecialAttackCooldownRemaining = mSpecialAttackCooldown;
    }
}

void Player::ChargeAttack(float deltaTime) {
    if (mAttackPressed && !mAttackPressedPrev) {
        mAttackPressTimer = mDefaultAttackPressTimer;
        GetGame()->GetAudioSystem()->PlaySE("chargingSE");
    } else if (mAttackPressed && mAttackPressedPrev) {
        float attackPressTimerPrev = mAttackPressTimer;
        mAttackPressTimer -= deltaTime;
        if (mAttackPressTimer >= 0.0f) {
            // mPos -= mFacingForwardVec * mChargeMoveSpeed * deltaTime;
            mPos -= -mUpVec * mChargeMoveSpeed * deltaTime;
        } else {
            mAttackPressTimer = 0.0f;
        }
        if (attackPressTimerPrev > 0.0f && mAttackPressTimer <= 0.0f) {
            GetGame()->GetAudioSystem()->PlaySE("chargedSE");
        }
    } else if (!mAttackPressed && mAttackPressedPrev && mAttackPressTimer <= 0.0f) {
        float pressTime = mDefaultAttackPressTimer - mAttackPressTimer / mDefaultAttackPressTimer;
        mStrongAttackTimer = mDefaultStrongAttackTimer * pressTime;
        mIsStrongAttack = true;
    } else if (!mAttackPressed && !mAttackPressedPrev && mAttackPressTimer >= 0.0f) {
        mAttackPressTimer = -1.0f;
    }
}

void Player::TakeDamage() {
    mDamageTimer = mDefaultDamageTimer;
    mInvincibleTimer = mDefaultInvincibleTimer;
    mIsDamaged = false;
    GetGame()->GetAudioSystem()->PlaySE("damagedSE");
}

void Player::Die() {
    mHp = mMaxHp;
    mPos = mRestartPos;
    mCurrentPlanetNum = mRestartPlanetIndex;
    mVelocity = {0.0f, 0.0f, 0.0f};
    mOnGround = true;
}

void Player::RideBoat() {
    std::vector<Boat*> boats = mCurrentPlanet->GetBoats();
    for (auto boat : boats) {
        glm::vec3 boatPos = boat->GetPos();
        bool boatIsActive = boat->GetIsActive();

        bool moving = boat->GetIsMoving();
        if (moving) {
            // ボートと一緒にプレイヤーを移動
            mPos = boatPos;
        } else if (!moving && boatIsActive) {
            float distToBoat = glm::length(mPos - boatPos);

            const float boatTouchRadius = 0.9f;
            if (distToBoat >= boatTouchRadius)
                continue;;
            
            boat->SetIsMoving(true);
            mIsActive = false;
        }
        
        // 到着処理
        float progress = boat->GetProgress();
        if (progress >= 1.0f)
        {
            mCurrentPlanetNum++;

            std::vector<Planet*> planets = GetGame()->GetCurrentStage()->GetPlanets();
            mCurrentPlanet = planets[mCurrentPlanetNum];

            glm::vec3 boatDestPos = boat->GetDestPos();
            mPos = boatDestPos;
            mVelocity = glm::vec3(0.0f);

            mOnGround = true;
            mIsActive = true;

            mRestartPos = boatDestPos;
            mRestartPlanetIndex = mCurrentPlanetNum;

            UpdateFallbackUpVec();
        }
    }
}

void Player::UpdateTimer(float deltaTime) {
    // 回避クールダウン消費
    if (mDodgeCooldown > 0.0f) {
        mDodgeCooldown -= deltaTime;
    }

    // ダメージを受けた際のノックバック
    if (mDamageTimer > 0.0f)
    {
        mDamageTimer -= deltaTime;
        glm::vec3 toPlayer = glm::normalize(mPos - mKnockBackFrom);
        mPos += toPlayer * deltaTime;
    }

    if (mSpecialAttackCooldownRemaining >= 0.0f)
    {
        mSpecialAttackCooldownRemaining -= deltaTime;
    }

    // 攻撃クールダウンタイム減少
    if (mAttackCooldownRemaining >= 0.0f)
    {
        mAttackCooldownRemaining -= deltaTime;
    }

    // 攻撃モーション
    if (mAttackMotionTimer >= 0.0f)
    {
        mAttackMotionTimer -= deltaTime;
        if (mAttackMotionTimer >= mDefaultAttackMotionTimer / 2.0f) {
            mPos += mFacingForwardVec * mAttackSpeed * deltaTime;
        }else {
            mPos -= mFacingForwardVec * mAttackSpeed * deltaTime;
        }
    }

    // 移動クールダウンタイム減少
    if (mAttackMoveLockRemaining > 0.0f)
    {
        mAttackMoveLockRemaining -= deltaTime;
    }

    if (mComboTimer > 0.0f) {
        mComboTimer -= deltaTime;
        if (mComboTimer <= 0.0f)
            mAttackIndex = 0; // コンボリセット
    }

    // 回避クールダウンタイム減少
    if (mAttackDodgeLockRemaining > 0.0f)
    {
        mAttackDodgeLockRemaining -= deltaTime;
    }

    if (mInvincibleTimer >= 0.0f) {
        mInvincibleTimer -= deltaTime;
    }

    if (mDodgeTimer > 0.0f) {
        mDodgeTimer -= deltaTime;
    }

    if (mStrongAttackTimer >= 0.0f)
    {
        mStrongAttackTimer -= deltaTime;
        // mPos = mPos + mFacingForwardVec * mStrongAttackSpeed * deltaTime;
        mPos = mPos + -mUpVec * mStrongAttackSpeed * deltaTime;
    }

    if (mRayCastTimer >= 0.0f) {
        mRayCastTimer -= deltaTime;
    }
}

void Player::UpdatePrev() {
    mAttackPressedPrev = mAttackPressed;
    mWideAttackPressedPrev = mWideAttackPressed;
    mDodgePressedPrev = mDodgePressed;
    mSpecialAttackPressedPrev = mSpecialAttackPressed;
    mIsDamagePrev = mIsDamaged;
}

glm::mat4 Player::getPlayerView(float cameraDistance, bool isFixed) {
    glm::vec3 toPosX;
    glm::vec3 cameraDir;

    if (isFixed) {
        toPosX = glm::normalize(-mFacingForwardVec);
        cameraDir = glm::normalize(std::cos(-0.2f) * toPosX + std::sin(-0.2f) * mCameraUpVec);
        mCameraPos = mCameraTargetPos - cameraDir * cameraDistance;
        glm::vec3 offset = glm::normalize(mCameraUpVec) * 1.0f;
        return glm::lookAt(mCameraPos, mCameraTargetPos + offset, mCameraUpVec);
    }

    toPosX = glm::normalize(-mForwardVec);
    cameraDir = glm::normalize(std::cos(mCameraPitch) * toPosX + std::sin(mCameraPitch) * mCameraUpVec);
    mCameraPos = mCameraTargetPos - cameraDir * cameraDistance;
    return glm::lookAt(mCameraPos, mCameraTargetPos, mCameraUpVec);
}