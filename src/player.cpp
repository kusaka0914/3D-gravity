#include "Player.h"
#include "Enemy.h"
#include "Boat.h"
#include "Key.h"
#include "Stage.h"
#include "Star.h"
#include "Game.h"
#include "PhysicsSystem.h"
#include "UIState.h"
#include "GameProgressState.h"
#include <btBulletDynamicsCommon.h>
#include <cmath>

Player::Player(Game* game)
    : Actor(game)
    , mCurrentPlanet(nullptr)
    , mPos(0.0f)
    , mUpVec(0.0f)
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
    , mMoveSpeed(3.2f)
    , mCameraStickX(0.0f)
    , mCameraStickY(0.0f)
    , mVelocity(0.0f, 0.0f, 0.0f)
    , mOnGround(true)
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
    , mAirAttackIndex(0)
    , mIsActive(true)
    , mAttackPressTimer(-1.0f)
    , mStrongAttackTimer(-1.0f)
    , mInvincibleTimer(-1.0f)
    , mComboTimer(-1.0f)
    , mSpecialAttackCooldownRemaining(-1.0f)
{
    
}

Player::~Player()
{
    
}

void Player::ProcessActor()
{   
    bool isPlaying = GetGame()->GetGameProgressState()->GetSceneState() == GameProgressState::SceneState::Playing;
    if(GetGame()->GetUIState()->GetIsUIActive() || !isPlaying) {
        mCanMove = false;
    }
    
    ProcessGameController();
    ProcessKeyboard();
}

void Player::UpdateActor(float deltaTime)
{
    UpdateCamera(deltaTime);

    UpdateWorldVec();

    if (!mIsDamaged && mAttackMoveLockRemaining <= 0.0f && mDodgeTimer <= 0.0f && mAttackPressTimer < 0.0f && mCanMove)
        UpdateWalk(deltaTime);

    UpdateDodge(deltaTime);

    bool isRising = glm::dot(mVelocity, mUpVec) > 0.2f;
    if (mDodgeTimer <= 0.0f && !isRising)
        DetermineLanding();

    if (mDodgeTimer <= 0.0f && mAttackPressTimer < 0.0f && mStrongAttackTimer <= 0.0f)
        ApplyGravity(deltaTime);

    if (mDodgeTimer <= 0.0f && mAttackMoveLockRemaining <= 0.0f && mAttackPressTimer < 0.0f && (std::abs(mMoveForward) > 0.01f || std::abs(mMoveLeft) > 0.01f))
        ChangeFaceDir();

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
    //         mFacingYaw = getYawFromDirection(up, moveDir) + 3.14159265f;
    //     }
    // }
    // mPos += mForwardVec * mMoveForward * dashSpeed * deltaTime;
    // mPos += mLeftVec * mMoveLeft * dashSpeed * deltaTime;
}

void Player::UpdateCamera(float deltaTime) {
    // カメラ更新
    const float cameraSensitivity = 2.5f;
    mCameraYaw += mCameraStickX * cameraSensitivity * deltaTime;
    mCameraPitch -= mCameraStickY * cameraSensitivity * deltaTime;
    mCameraPitch = glm::clamp(mCameraPitch, -1.2f, -0.2f);
}

void Player::UpdateWorldVec() {
    if (mCurrentPlanet->GetPlanetType() == Planet::PlanetType::Normal) {
        mUpVec = {0.0f, 1.0f, 0.0f};
    } else {
        mUpVec = glm::normalize(mPos - mCurrentPlanet->GetCenter());
    }

    glm::vec3 worldLeft = glm::cross(mUpVec, glm::vec3(0, 0, 1));
    if (glm::length(worldLeft) < 0.01f)
        worldLeft = glm::normalize(glm::cross(mUpVec, glm::vec3(0, 1, 0)));
    else 
        worldLeft = glm::normalize(worldLeft);

    mForwardVec = glm::normalize(glm::cross(worldLeft, mUpVec) * std::cos(mCameraYaw) - std::sin(mCameraYaw) * worldLeft);
    mLeftVec = glm::normalize(glm::cross(mUpVec, mForwardVec));

    mFacingForwardVec = glm::normalize(glm::cross(worldLeft, mUpVec) * std::cos(mFacingYaw) - std::sin(mFacingYaw) * worldLeft);
    mFacingLeftVec = glm::normalize(glm::cross(mUpVec, mFacingForwardVec));
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
        mVelocity += mUpVec * 5.0f;
        mOnGround = false;
        GetGame()->GetAudioSystem()->PlaySE("jumpSE");
    }
}

void Player::UpdateDodge(float deltaTime) {
    const float dodgeDuration = 0.05f;
    const float dodgeCooldownTime = 1.0f;
    // 向いている方向へ回避開始
    if (mDodgePressed && !mDodgePressedPrev && mDodgeCooldown <= 0.0f && mAttackDodgeLockRemaining <= 0.0f)
        StartDodge(dodgeDuration, dodgeCooldownTime);
    
    // 回避中移動
    if (mDodgeTimer > 0.0f)
        Dodge(deltaTime, dodgeDuration);
}

void Player::StartDodge(float dodgeDuration, float dodgeCooldownTime) {
    mDodgeDir = -mFacingForwardVec;
    mDodgeTimer = dodgeDuration;
    mDodgeCooldown = dodgeCooldownTime;
    mDodgeStartHeight = glm::length(mPos - mCurrentPlanet->GetCenter());
    mVelocity = glm::vec3(0.0f);
    GetGame()->GetAudioSystem()->PlaySE("dodgeSE");
}

void Player::Dodge(float deltaTime, float dodgeDuration) {
    const float dodgeDistance = 2.0f;
    float dodgeSpeed = dodgeDistance / dodgeDuration;
    glm::vec3 moveDelta = mDodgeDir * dodgeSpeed * deltaTime;
    glm::vec3 desiredPos = mPos + moveDelta;
    
    desiredPos = GetGame()->GetPhysicsSystem()->CheckCollision(moveDelta, desiredPos);
    mPos = desiredPos;

    glm::vec3 center = mCurrentPlanet->GetCenter();
    // 空中回避：直前の高さを維持して浮遊
    float dist = glm::length(mPos - center);
    if (dist > 1e-6f)
        mPos = center + (mPos - center) / dist * mDodgeStartHeight;
}

void Player::DetermineLanding() {
    bool meshGround = false;
    glm::vec3 center = mCurrentPlanet->GetCenter();

    glm::vec3 rayFromPos = mPos + mUpVec * 0.1f;
    glm::vec3 rayToPos = mPos - mUpVec * 0.1f;
    btVector3 rayFrom(rayFromPos.x, rayFromPos.y, rayFromPos.z);
    btVector3 rayTo(rayToPos.x, rayToPos.y, rayToPos.z);

    btCollisionWorld::ClosestRayResultCallback rayCallback(rayFrom, rayTo);
    PhysicsSystem* physics = GetGame()->GetPhysicsSystem();
    btDiscreteDynamicsWorld* bulletWorld = physics ? physics->GetBulletWorld() : nullptr;
    bulletWorld->rayTest(rayFrom, rayTo, rayCallback);
    // 着地処理
    if (rayCallback.hasHit())
    {
        btVector3 hitPt = rayCallback.m_hitPointWorld;
        glm::vec3 hitPos(hitPt.x(), hitPt.y(), hitPt.z());
        
        mPos = hitPos;
        mOnGround = true;
        mVelocity = glm::vec3(0.0f);
        meshGround = true;
    }
    // 落下開始
    if (!meshGround && mOnGround)
    {
        mOnGround = false;
    }
}

void Player::ApplyGravity(float deltaTime) {
    mVelocity -= mUpVec * 9.8f * deltaTime;
    mPos += mVelocity * deltaTime;

    if (mCurrentPlanet->GetPlanetType() == Planet::PlanetType::Normal)
        return;
    
    glm::vec3 center = mCurrentPlanet->GetCenter();
    float radius = mCurrentPlanet->GetRadius();
    float dist = glm::length(mPos - center);
    // 落下して惑星内部にめり込んだらリスタート地点へ
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
    glm::vec3 moveDir = mForwardVec * mMoveForward + mLeftVec * mMoveLeft;
    float len = glm::length(moveDir);
    if (len > 0.001f)
    {
        // 移動方向を正規化
        moveDir /= len;
        mFacingYaw = getYawFromDirection(mUpVec, moveDir) + 3.14159265f;
    }
}

void Player::Attack(float deltaTime) {
    mAttackStartHeight = glm::length(mPos - mCurrentPlanet->GetCenter());
    mVelocity = glm::vec3(0.0f);

    auto applyAttackLocksFromCooldown = [this]() {
        mAttackMoveLockRemaining = std::min(mAttackCooldownRemaining, 0.5f) + 0.2f;
        mAttackDodgeLockRemaining = std::max(0.0f, mAttackMoveLockRemaining - 0.5f);
        mComboTimer = mAttackMoveLockRemaining + 1.0f;
        if (!mOnGround)
        {
            // mAttackCooldownRemaining = 0.2f;
            // mAirAttackIndex++;
            // if (mAirAttackIndex == 5) {
            //     mAirAttackIndex = 0;
            // }
        } else {
            mAttackMotionTimer = 0.3f;
            mAirAttackIndex = 0;
        }
    };
    
    float attackRange;
    float attackAngle;
    if (mAttackPressed) {
        attackRange = 1.0f;
        attackAngle = 0.8f;
        mAttack = 10;
    } else if (mWideAttackPressed) {
        attackRange = 2.8f;
        attackAngle = 0.4f;
        mAttack = 5;
    } else if (mStrongAttackTimer >= 0.0f) {
        attackRange = 6.0f;
        mAttack = 50;
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
        float dot = glm::dot(-mFacingForwardVec, toEnemy);
        float effectiveRange = attackRange + enemy->GetRadius();
        if (mStrongAttackTimer >= 0.0f && dist <= effectiveRange) {
            hitEnemies.push_back(enemy);
        }
        if (dist <= effectiveRange && dot >= attackAngle)
            hitEnemies.push_back(enemy);
    }

    if (!hitEnemies.empty())
    {
        for (Enemy* enemy : hitEnemies)
            enemy->SetIsDamaged(true);

        mAttackCooldownRemaining = 0.3f;
        mAttackIndex++;
        float strongAttackTimerNext = mStrongAttackTimer - deltaTime;
        if (mStrongAttackTimer >= 0.0f) {
            for (Enemy* enemy : hitEnemies) {
                enemy->SetIsStrongAttacked(true);
            }
            GetGame()->GetAudioSystem()->PlaySE("attackAirSE");
        }
        if (mAttackIndex == 3)
        {
            mAttackCooldownRemaining = 1.0f;
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
        mAttackCooldownRemaining = 0.3f;
        mAttackIndex++;
        if (mAttackIndex == 3)
        {
            mAttackCooldownRemaining = 1.0f;
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
        mAttack = 10;
        enemy->SetIsDamaged(true);
        if (enemy->GetOnGround()) {
            enemy->SetIsBroken(true);
        }
        mSpecialAttackCooldownRemaining = 30.0f;
    }
}

void Player::ChargeAttack(float deltaTime) {
    if (mAttackPressed && !mAttackPressedPrev) {
        mAttackPressTimer = 0.5f;
        GetGame()->GetAudioSystem()->PlaySE("chargingSE");
    } else if (mAttackPressed && mAttackPressedPrev) {
        float attackPressTimerPrev = mAttackPressTimer;
        mAttackPressTimer -= deltaTime;
        if (mAttackPressTimer >= 0.0f) {
            mPos -= -mFacingForwardVec * 6.0f * deltaTime;
            mPos -= -mUpVec * 6.0f * deltaTime;
        } else {
            mAttackPressTimer = 0.0f;
        }
        if (attackPressTimerPrev > 0.0f && mAttackPressTimer <= 0.0f) {
            GetGame()->GetAudioSystem()->PlaySE("chargedSE");
        }
    } else if (!mAttackPressed && mAttackPressedPrev && mAttackPressTimer <= 0.0f) {
        mStrongAttackTimer = 0.03f;
    } else if (!mAttackPressed && !mAttackPressedPrev && mAttackPressTimer >= 0.0f) {
        mAttackPressTimer = -1.0f;
    }
}

void Player::TakeDamage() {
    mDamageTimer = 1.0f;
    mInvincibleTimer = 2.0f;
    mIsDamaged = false;
    GetGame()->GetAudioSystem()->PlaySE("damagedSE");
}

void Player::Die() {
    mHp = 100;
    mPos = mRestartPos;
    mCurrentPlanetNum = mRestartPlanetIndex;
    mVelocity = {0.0f, 0.0f, 0.0f};
    mOnGround = true;
}

void Player::RideBoat() {
    std::vector<Boat*> boats = mCurrentPlanet->GetBoats();
    for (auto boat : boats) {
        if (!boat->GetIsMoving() && boat->GetIsActive() && mCurrentPlanetNum == boat->GetCurrentPlanetNum())
        {
            float distToBoat = glm::length(mPos - boat->GetPos());
            const float boatTouchRadius = 1.8f;
            if (distToBoat < boatTouchRadius)
            {
                boat->SetIsMoving(true);
                mIsActive = false;
            }
        }
        bool moving = boat->GetIsMoving();
        if (moving) {
            // ボートと一緒にプレイヤーを移動
            mPos = boat->GetPos();
        }
        float progress = boat->GetProgress();
        // 到着処理
        std::vector<Planet*> planets = GetGame()->GetCurrentStage()->GetPlanets();
        if (progress >= 1.0f)
        {
            mCurrentPlanetNum = boat->GetDestPlanet();
            mCurrentPlanet = planets[mCurrentPlanetNum];
            mPos = boat->GetDestPos();
            mOnGround = true;
            mVelocity = glm::vec3(0.0f);
            mRestartPos = boat->GetDestPos();
            mRestartPlanetIndex = boat->GetDestPlanet();
            mIsActive = true;
            glm::vec3 boatUpVec = boat->GetUpVec();
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
        if (mAttackMotionTimer >= 0.15f) {
            mPos += -mFacingForwardVec * 5.0f * deltaTime;
        }else {
            mPos -= -mFacingForwardVec * 5.0f * deltaTime;
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
        mPos += -mFacingForwardVec * 100.0f * deltaTime;
        mPos += -mUpVec * 100.0f * deltaTime;
    }
}

void Player::UpdatePrev() {
    mAttackPressedPrev = mAttackPressed;
    mWideAttackPressedPrev = mWideAttackPressed;
    mDodgePressedPrev = mDodgePressed;
    mSpecialAttackPressedPrev = mSpecialAttackPressed;
    mIsDamagePrev = mIsDamaged;
}

float Player::getYawFromDirection(const glm::vec3& mUpVec, const glm::vec3& dir) {
    glm::vec3 worldLeft = glm::normalize(glm::cross(mUpVec, glm::vec3(0, 0, 1)));
    if (glm::length(worldLeft) < 0.01f)
        worldLeft = glm::normalize(glm::cross(mUpVec, glm::vec3(1, 0, 0)));
    glm::vec3 right = glm::cross(worldLeft, mUpVec);
    return std::atan2(-glm::dot(dir, worldLeft), glm::dot(dir, right));
}

glm::mat4 Player::getPlayerView(float cameraDistance, bool isFixed) {
    glm::vec3 toPosX;
    glm::vec3 cameraDir;
    glm::vec3 cameraPos;
    if (isFixed) {
        toPosX = glm::normalize(mFacingForwardVec);
        cameraDir = glm::normalize(std::cos(-0.2f) * toPosX + std::sin(-0.2f) * mUpVec);  
        cameraPos = mPos - cameraDir * cameraDistance;
        glm::vec3 offset = glm::normalize(mUpVec) * 1.0f;
        return glm::lookAt(cameraPos, mPos + offset, mUpVec);  
    }

    toPosX = glm::normalize(-mForwardVec);
    cameraDir = glm::normalize(std::cos(mCameraPitch) * toPosX + std::sin(mCameraPitch) * mUpVec);
    cameraPos = mPos - cameraDir * cameraDistance;
    return glm::lookAt(cameraPos, mPos, mUpVec);
}