#include "Player.h"
#include "Enemy.h"
#include "Boat.h"
#include "Key.h"
#include "Stage.h"
#include "Star.h"
#include "Game.h"
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
    , mMoveSpeed(1.0f)
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
    , mCounterPressed(false)
    , mCounterPressedPrev(false)
    , mDamageTimer(0.0f)
    , mAttackCooldownRemaining(0.0f)
    , mAttackMoveLockRemaining(0.0f)
    , mAttackDodgeLockRemaining(0.0f)
    , mAttackHeightLockRemaining(-5.0f)
    , mAttackIndex(0)
    , mAirAttackIndex(0)
{
    Stage* currentStage = GetGame()->GetCurrentStage();
    mCurrentPlanet = currentStage->GetPlanets()[0];

    glm::vec3 center = mCurrentPlanet->GetCenter();
    float radius = mCurrentPlanet->GetRadius();
    mPos = center + glm::vec3(10.0f, radius, 0.0f);

    mRestartPos = mPos;
    mRestartPlanetIndex = mCurrentPlanetNum;
}

Player::~Player()
{
    
}

void Player::ProcessActor()
{
    // ゲームパッド対応（1P）
    // 入力をSDLに取り込む
    SDL_PumpEvents();
    // コントローラーの状態更新
    SDL_GameControllerUpdate();
    SDL_GameController* sdlController = GetGame()->GetSdlController();
    if (sdlController && SDL_GameControllerGetAttached(sdlController) && mPlayerNum == 1 && mDamageTimer <= 0.0f)
    {
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
        if (SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_X) && mAirAttackIndex == 0)
            mAttackPressed = true;

        // 回避（Bボタン）
        mDodgePressed = false;
        if (SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_B))
            mDodgePressed = true;

        // カウンター（Lボタン）
        mCounterPressed = false;
        if (SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_LEFTSHOULDER))
            mCounterPressed = true;

        // ダッシュ判定（Rボタン）
        mMoveSpeed = 2.0f;
        if (SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER))
            mMoveSpeed = 3.0f;
    }

    if (GetGame()->GetIsPlayer2Joined() && mPlayerNum != 1)
    {
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
            mCounterPressed = true;
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
}

void Player::UpdateActor(float deltaTime)
{
    // カメラ更新
    const float cameraSensitivity = 2.5f;
    mCameraYaw += mCameraStickX * cameraSensitivity * deltaTime;
    mCameraPitch -= mCameraStickY * cameraSensitivity * deltaTime;
    mCameraPitch = glm::clamp(mCameraPitch, -1.2f, -0.2f);

    std::vector<Enemy*> enemies = mCurrentPlanet->GetEnemies();

    mUpVec = glm::normalize(mPos - mCurrentPlanet->GetCenter());
    glm::vec3 worldLeft = glm::normalize(glm::cross(mUpVec, glm::vec3(0, 0, 1)));
    if (glm::length(worldLeft) < 0.01f)
        worldLeft = glm::normalize(glm::cross(mUpVec, glm::vec3(1, 0, 0)));
    mForwardVec = glm::normalize(glm::cross(worldLeft, mUpVec) * std::cos(mCameraYaw) - std::sin(mCameraYaw) * worldLeft);
    mLeftVec = glm::normalize(glm::cross(mUpVec, mForwardVec));

    mFacingForwardVec = glm::normalize(glm::cross(worldLeft, mUpVec) * std::cos(mFacingYaw) - std::sin(mFacingYaw) * worldLeft);
    mFacingLeftVec = glm::normalize(glm::cross(mUpVec, mFacingForwardVec));

    // プレイヤー移動（ダメージ時、空中固定時、回避時以外）
    if (!mIsDamaged && mAttackMoveLockRemaining <= 0.0f && mDodgeTimer <= 0.0f)
    {
        glm::vec3 moveDelta = mForwardVec * mMoveForward * mMoveSpeed * deltaTime + mLeftVec * mMoveLeft * mMoveSpeed * deltaTime;
        glm::vec3 desiredPos = mPos + moveDelta;

        glm::vec3 center = mCurrentPlanet->GetCenter();
        float planetRadius = mCurrentPlanet->GetRadius();

        // 敵との当たり判定
        for (auto* enemy : enemies)
        {
            if (!enemy->GetIsAlive())
                continue;
            glm::vec3 ePos = enemy->GetPos();
            glm::vec3 toDesired = desiredPos - ePos;
            float d = glm::length(toDesired);
            float minDist = enemy->GetRadius();
            if (d < minDist && d > 1e-5f)
                desiredPos = ePos + (toDesired / d) * minDist;
        }
        // 壁当たり：球スイープで移動経路に障害があれば移動を打ち切り
        // if (bulletOk && bulletWorld && bulletWallSphere && glm::length(moveDelta) > 1e-5f)
        // {
        //     glm::vec3 upForSweep = glm::normalize(players[0].pos - planets[players[0].planetIndex].center);
        //     glm::vec3 sweepFrom = players[0].pos + upForSweep * 0.4f; // 腰高で判定（地面に当たりにくくする）
        //     glm::vec3 sweepTo = desiredPos + upForSweep * 0.4f;
        //     btTransform fromBt, toBt;
        //     fromBt.setIdentity();
        //     fromBt.setOrigin(btVector3(sweepFrom.x, sweepFrom.y, sweepFrom.z));
        //     toBt.setIdentity();
        //     toBt.setOrigin(btVector3(sweepTo.x, sweepTo.y, sweepTo.z));
        //     btVector3 sweepFromBt(sweepFrom.x, sweepFrom.y, sweepFrom.z);
        //     btVector3 sweepToBt(sweepTo.x, sweepTo.y, sweepTo.z);
        //     btCollisionWorld::ClosestConvexResultCallback sweepCallback(sweepFromBt, sweepToBt);
        //     bulletWorld->convexSweepTest(bulletWallSphere, fromBt, toBt, sweepCallback);
        //     if (sweepCallback.hasHit())
        //     {
        //         // 壁手前で一度止め、残りを壁に沿う方向（スライド）に投影して進める
        //         float allowFrac = std::max(0.0f, sweepCallback.m_closestHitFraction - 0.02f);
        //         glm::vec3 posAfterHit = players[0].pos + moveDelta * allowFrac;
        //         glm::vec3 hitNormGlm(
        //             sweepCallback.m_hitNormalWorld.x(),
        //             sweepCallback.m_hitNormalWorld.y(),
        //             sweepCallback.m_hitNormalWorld.z());
        //         // 阻害された移動を壁面に投影 → 壁沿いのスライドベクトル
        //         glm::vec3 blocked = moveDelta * (1.0f - allowFrac);
        //         glm::vec3 slideVec = blocked - hitNormGlm * glm::dot(blocked, hitNormGlm);
        //         const float slideEps = 1e-4f;
        //         if (glm::length(slideVec) > slideEps)
        //         {
        //             glm::vec3 slideFrom = posAfterHit + upForSweep * 0.4f;
        //             glm::vec3 slideTo = slideFrom + slideVec;
        //             btTransform fromBt2, toBt2;
        //             fromBt2.setIdentity();
        //             fromBt2.setOrigin(btVector3(slideFrom.x, slideFrom.y, slideFrom.z));
        //             toBt2.setIdentity();
        //             toBt2.setOrigin(btVector3(slideTo.x, slideTo.y, slideTo.z));
        //             btVector3 sFrom(slideFrom.x, slideFrom.y, slideFrom.z);
        //             btVector3 sTo(slideTo.x, slideTo.y, slideTo.z);
        //             btCollisionWorld::ClosestConvexResultCallback slideCallback(sFrom, sTo);
        //             bulletWorld->convexSweepTest(bulletWallSphere, fromBt2, toBt2, slideCallback);
        //             float slideAllow = slideCallback.hasHit()
        //                                 ? std::max(0.0f, slideCallback.m_closestHitFraction - 0.02f)
        //                                 : 1.0f;
        //             desiredPos = posAfterHit + slideVec * slideAllow;
        //         }
        //         else
        //         {
        //             desiredPos = posAfterHit;
        //         }
        //     }
        // }
        mPos = desiredPos;
        // ジャンプ処理
        if (mOnGround && mJumpPressed)
        {
            mVelocity += mUpVec * 5.0f;
            mOnGround = false;
        }
    }

    const float dodgeDuration = 0.2f;
    const float dodgeCooldownTime = 1.0f;
    // 向いている方向へ回避開始
    if (mDodgePressed && !mDodgePressedPrev && mDodgeCooldown <= 0.0f && mDodgeTimer <= 0.0f && mAttackDodgeLockRemaining <= 0.0f)
    {
        mDodgeDir = -mFacingForwardVec;
        mDodgeTimer = dodgeDuration;
        mDodgeCooldown = dodgeCooldownTime;
        mDodgeStartHeight = glm::length(mPos - mCurrentPlanet->GetCenter());
        mVelocity = glm::vec3(0.0f);
    }
    // 回避中移動
    if (mDodgeTimer > 0.0f)
    {
        const float dodgeDistance = 1.5f;
        float dodgeSpeed = dodgeDistance / dodgeDuration;
        glm::vec3 moveDelta = mDodgeDir * dodgeSpeed * deltaTime;
        glm::vec3 desiredPos = mPos + moveDelta;
        for (auto enemy : enemies) {
            glm::vec3 ePos = enemy->GetPos();
            glm::vec3 toDesired = desiredPos - ePos;
            float d = glm::length(toDesired);
            float minDist = enemy->GetRadius();
            if (d < minDist && d > 1e-5f)
                desiredPos = ePos + (toDesired / d) * minDist;
        }
        
        mPos = desiredPos;
        glm::vec3 center = mCurrentPlanet->GetCenter();
        // 空中回避：直前の高さを維持して浮遊
        float dist = glm::length(mPos - center);
        if (dist > 1e-6f)
            mPos = center + (mPos - center) / dist * mDodgeStartHeight;
        mDodgeTimer -= deltaTime;
    }
    // 回避クールダウン消費
    if (mDodgeCooldown > 0.0f && mAttackHeightLockRemaining <= 0.0f) {
        mDodgeCooldown -= deltaTime;
    }

    // 攻撃時の高さを維持して浮遊
    if (mAttackHeightLockRemaining > 0.0f)
    {
        glm::vec3 center = mCurrentPlanet->GetCenter();
        float dist = glm::length(mPos - center);
        if (dist > 1e-6f)
            mPos = center + (mPos - center) / dist * mAttackStartHeight;
    }
    // Bullet レイキャスト：足元にメッシュがあれば地形に沿わせ、穴の上なら重力で落ちる
    // 回避中・攻撃硬直中は浮遊のためスキップ。上昇中（ジャンプ直後）もスキップし、地上 or 落下中のみ判定
    // glm::vec3 upForJump = glm::normalize(players[0].pos - planets[players[0].planetIndex].center);
    // bool isRising = glm::dot(players[0].velocity, upForJump) > 0.5f;
    // bool meshGround = false;
    // if (mDodgeTimer <= 0.0f && mAttackHeightLockRemaining <= 0.0f && bulletOk && bulletWorld && !isRising)
    // {
    //     glm::vec3 center = planets[players[0].planetIndex].center;
    //     glm::vec3 upDir = glm::normalize(players[0].pos - center);
    //     glm::vec3 rayFrom3 = players[0].pos + upDir * 0.1f;
    //     glm::vec3 rayTo3 = players[0].pos - upDir * 0.1f;
    //     btVector3 rayFrom(rayFrom3.x, rayFrom3.y, rayFrom3.z);
    //     btVector3 rayTo(rayTo3.x, rayTo3.y, rayTo3.z);
    //     btCollisionWorld::ClosestRayResultCallback rayCallback(rayFrom, rayTo);
    //     bulletWorld->rayTest(rayFrom, rayTo, rayCallback);
    //     if (rayCallback.hasHit())
    //     {
    //         btVector3 hitPt = rayCallback.m_hitPointWorld;
    //         glm::vec3 hitPos(hitPt.x(), hitPt.y(), hitPt.z());
    //         float hitDist = glm::length(hitPos - center);
    //         float playerDist = glm::length(players[0].pos - center);
    //         if (playerDist - hitDist < 2.0f)
    //         {
    //             players[0].pos = hitPos;
    //             players[0].onGround = true;
    //             players[0].velocity = glm::vec3(0.0f);
    //             meshGround = true;
    //         }
    //     }
    //     if (!meshGround && players[0].onGround)
    //     {
    //         players[0].onGround = false;
    //     }
    // }

    // 回避中・空中攻撃中は重力をかけず、終了後に通常通り落下
    if (mDodgeTimer <= 0.0f && mAttackHeightLockRemaining <= 0.0f) {
        glm::vec3 center = mCurrentPlanet->GetCenter();
        float radius = mCurrentPlanet->GetRadius();
        mUpVec = glm::normalize(mPos - center);
    
        // bool skipGroundSnap = bulletOk && bulletWorld;
        bool skipGroundSnap = false; // TODO:
        if (mOnGround && !skipGroundSnap) {
            mPos = center + glm::normalize(mPos - center) * radius;
        }

        // 重力処理
        mVelocity -= mUpVec * 9.8f * deltaTime;
        mPos += mVelocity * deltaTime;

        // 落下時に初期位置に移動
        float dist = glm::length(mPos - center);
        if (!skipGroundSnap && dist <= radius) {
            mOnGround = true;
            mPos = center + glm::normalize(mPos - center) * radius;
            mVelocity = glm::vec3(0, 0, 0);
        }
    }

    // 落下して惑星内部にめり込んだらリスタート地点へ
    // if (bulletOk && bulletWorld)
    // {
    //     float dist = glm::length(players[0].pos - planets[players[0].planetIndex].center);
    //     float r = planets[players[0].planetIndex].radius;
    //     if (dist < r * 0.5f)
    //     {
    //         players[0].pos = restartPos;
    //         players[0].planetIndex = restartPlanetIndex;
    //         players[0].velocity = glm::vec3(0.0f);
    //         players[0].onGround = true;
    //     }
    // }

    // スティックを倒した方向を向く。移動ロック中は地上のみ向き固定、空中攻撃中は向き替え可
    if ((mAttackMoveLockRemaining <= 0.0f) && (std::abs(mMoveForward) > 0.01f || std::abs(mMoveLeft) > 0.01f))
    {
        glm::vec3 moveDir = mForwardVec * mMoveForward + mLeftVec * mMoveLeft;
        float len = glm::length(moveDir);
        if (len > 0.001f)
        {
            // 移動方向を正規化
            moveDir /= len;
            mFacingYaw = getYawFromDirection(mUpVec, moveDir) + 3.14159265f;
        }
    }

    // 攻撃
    const float attackRange = 1.8f;
    const float attackAngle = 0.8f;
    bool canAttack = mAttackPressed && !mAttackPressedPrev && mAttackCooldownRemaining <= 0.0f;
    if (canAttack || (mAirAttackIndex > 0 && mAttackMotionTimer < 0))
    {
        mAttackStartHeight = glm::length(mPos - mCurrentPlanet->GetCenter());
        mVelocity = glm::vec3(0.0f);
        mAttackMotionTimer = 0.3f;

        auto applyAttackLocksFromCooldown = [this]() {
            mAttackMoveLockRemaining = std::min(mAttackCooldownRemaining, 0.5f) + 0.2f;
            mAttackDodgeLockRemaining = std::max(0.0f, mAttackMoveLockRemaining - 0.5f);
            if (!mOnGround)
            {
                mAttackCooldownRemaining = 0.2f;
                mAirAttackIndex++;
                if (mAirAttackIndex == 5) {
                    mAirAttackIndex = 0;
                }
                if (mAttackHeightLockRemaining <= -1.0f)
                    mAttackHeightLockRemaining = 1.5f;
            } else {
                mAirAttackIndex = 0;
            }
        };

        std::vector<Enemy*> hitEnemies;
        for (auto& enemy : enemies)
        {
            if (!enemy->GetIsAlive())
                continue;
            glm::vec3 enemyPos = enemy->GetPos();
            glm::vec3 toEnemy = glm::normalize(enemyPos - mPos);
            float dist = glm::length(enemyPos - mPos);
            float dot = glm::dot(-mFacingForwardVec, toEnemy);
            float effectiveRange = attackRange + enemy->GetRadius();
            if (dist <= effectiveRange && dot >= attackAngle)
                hitEnemies.push_back(enemy);
        }

        if (!hitEnemies.empty())
        {
            for (Enemy* enemy : hitEnemies)
                enemy->SetIsDamaged(true);

            mAttackCooldownRemaining = 0.3f;
            mAttackIndex++;
            if (mAttackIndex == 3)
            {
                mAttackIndex = 0;
                for (Enemy* enemy : hitEnemies)
                {
                    if (enemy->GetOnGround())
                        enemy->SetIsLaunched(true);
                }
            }
            applyAttackLocksFromCooldown();
            GetGame()->GetAudioSystem()->PlaySE("attackSE");
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

    // カウンター
    if (mCounterPressed && !mCounterPressedPrev)
    {
        for (auto& enemy : enemies)
        {
            if (mCounterCooldownRemaining > 0.0f)
                continue;
            glm::vec3 enemyPos = enemy->GetPos();
            float enemyStandByAttackTimer = enemy->GetStandByAttackTimer() ;
            if (!enemy->GetIsAlive())
                continue;
            if (enemyStandByAttackTimer > 0.5f){
                mCounterCooldownRemaining = 0.5f;
            }
            if (enemyStandByAttackTimer <= 0.3f || enemyStandByAttackTimer > 0.5f)
                continue;
                
            glm::vec3 toEnemy = glm::normalize(enemyPos - mPos);
            float dist = glm::length(enemyPos - mPos);
            float dot = glm::dot(-mFacingForwardVec, toEnemy);
            float effectiveRange = attackRange + enemy->GetRadius();
            bool isHit = dist <= effectiveRange && dot >= attackAngle;
            if (isHit)
            {
                enemy->SetIsCountered(true);
                GetGame()->GetAudioSystem()->PlaySE("counterSE");
                mAttackMotionTimer = 0.3f;
            }
        }
    }

    if (mCounterCooldownRemaining >= 0.0f)
    {
        mCounterCooldownRemaining -= deltaTime;
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
        if (mAttackMoveLockRemaining < 0.0f)
            mAttackMoveLockRemaining = 0.0f;
        if (mAttackMoveLockRemaining <= 0.0f)
            mAttackIndex = 0; // 歩けるようになったらコンボリセット
    }

    // 回避クールダウンタイム減少
    if (mAttackDodgeLockRemaining > 0.0f)
    {
        mAttackDodgeLockRemaining -= deltaTime;
        if (mAttackDodgeLockRemaining < 0.0f)
            mAttackDodgeLockRemaining = 0.0f;
    }

    // 空中浮遊クールダウンタイム減少
    mAttackHeightLockRemaining -= deltaTime;

    // ダメージを受けた際のノックバック
    if (mDamageTimer > 0.0f)
    {
        glm::vec3 toPlayer = glm::normalize(mPos - mKnockBackFrom);
        mPos += toPlayer * deltaTime;
        mDamageTimer -= deltaTime;
    }
    else
    {
        mIsDamaged = false;
    }

    std::vector<Boat*> boats = mCurrentPlanet->GetBoats();
    Key* key = mCurrentPlanet->GetKey();
    // ボートに触れたら到着先へ移動開始（ボートが出現した惑星にいる時のみ）
    for (auto boat : boats) {
        if (!boat->GetIsMoving() && key->GetCollectableComponent()->GetIsObtained() && mCurrentPlanetNum == boat->GetCurrentPlanetNum())
        {
            float distToBoat = glm::length(mPos - boat->GetPos());
            const float boatTouchRadius = 1.8f;
            if (distToBoat < boatTouchRadius)
            {
                boat->SetIsMoving(true);
            }
        }
    }

    // ボート移動
    for (auto boat : boats) {
        bool moving = boat->GetIsMoving();
        if (moving) {
            // ボートと一緒にプレイヤーを移動
            glm::vec3 boatPos = boat->GetPos();
            glm::vec3 boatUpVec = boat->GetUpVec();
            const float playerHeightAboveBoat = 0.7f;
            mPos = boatPos + boatUpVec * playerHeightAboveBoat;
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
            // if (bulletGhost)
            // {
            //     btTransform t;
            //     t.setIdentity();
            //     t.setOrigin(btVector3(players[0].pos.x, players[0].pos.y, players[0].pos.z));
            //     bulletGhost->setWorldTransform(t);
            // }
        }
    }

    // スターに触れたらゲームクリア
    Star* star = mCurrentPlanet->GetStar();
    if (star->GetIsActive())
    {
        float distToStar = glm::length(mPos - star->GetPos());
        const float starTouchRadius = 1.5f;
        if (distToStar < starTouchRadius)
        {
            star->SetIsActive(false);
            Mix_HaltMusic();
            GetGame()->GetAudioSystem()->PlaySE("clearSE");
            std::cout << "Game Clear!" << std::endl;
        }
    }

    mAttackPressedPrev = mAttackPressed;
    mDodgePressedPrev = mDodgePressed;
    mCounterPressedPrev = mCounterPressed;
    mIsDamagePrev = mIsDamaged;
}

float Player::getYawFromDirection(const glm::vec3& mUpVec, const glm::vec3& dir) {
    glm::vec3 worldLeft = glm::normalize(glm::cross(mUpVec, glm::vec3(0, 0, 1)));
    if (glm::length(worldLeft) < 0.01f)
        worldLeft = glm::normalize(glm::cross(mUpVec, glm::vec3(1, 0, 0)));
    glm::vec3 right = glm::cross(worldLeft, mUpVec);
    return std::atan2(-glm::dot(dir, worldLeft), glm::dot(dir, right));
}

glm::mat4 Player::getPlayerView() {
    const float cameraDistance = 12.0f;
    glm::vec3 back = glm::normalize(-mForwardVec);
    glm::vec3 cameraDir = glm::normalize(std::cos(mCameraPitch) * back + std::sin(mCameraPitch) * mUpVec);
    glm::vec3 cameraPos = mPos - cameraDir * cameraDistance;
    return glm::lookAt(cameraPos, mPos, mUpVec);
}