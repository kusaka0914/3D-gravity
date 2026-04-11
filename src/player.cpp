#include "Player.h"
#include "Enemy.h"
#include "Boat.h"
#include "Stage.h"
#include "Game.h"
#include <cmath>

Player::Player(Game* game)
    : Actor(game)
    , mPos(0.0f)
    , mKnockBackFrom(0.0f)
    , mRestartPos(0.0f)
    , mCurrentPlanet(0)
    , mCameraYaw(0.0f)
    , mCameraPitch(0.4f)
    , mFacingYaw(0.0f)
    , mVelocity(0.0f, 0.0f, 0.0f)
    , mOnGround(true)
    , mAttack(0.0f)
    , mHp(0.0f)
    , mIsDamaged(false)
    , mIsDamagePrev(false)
    , mDamageTimer(0.0f)
    , mAttackCooldownRemaining(0.0f)
    , mAttackMoveLockRemaining(0.0f)
    , mAttackDodgeLockRemaining(0.0f)
    , mAttackIndex(0)
    , mRestartPlanetIndex(0)
{
    
}

Player::~Player()
{
    
}

void Player::UpdateActor(float deltaTime)
{
    // int currentStageNum = GetGame()->GetCurrentStageNum();
    // Stage* currentStage = GetGame()->GetStages()[currentStageNum];
    // Planet* currentPlanet = currentStage->GetPlanets()[mCurrentPlanet];
    // const auto& seList = GetGame()->GetSEList();
    // std::vector<Enemy*> enemies = currentPlanet->GetEnemies();

    // mUpVec = glm::normalize(mPos - currentPlanet->GetCenter());

    // // ボート移動
    // std::vector<Boat*> boats = currentPlanet->GetBoats();
    // for (auto boat : boats) {
    //     bool active = boat->GetIsActive();
    //     if (active) {
    //         // ボートと一緒にプレイヤーを移動
    //         glm::vec3 boatPos = boat->GetPos();
    //         glm::vec3 boatUpVec = boat->GetUpVec();
    //         const float playerHeightAboveBoat = 0.7f;
    //         mPos = boatPos + boatUpVec * playerHeightAboveBoat;

    //         // 到着処理
    //         float progress = boat->GetProgress();
    //         if (progress >= 1.0f)
    //         {
    //             mCurrentPlanet = boat->GetDestPlanet();
    //             mPos = boat->GetDestPos();
    //             mOnGround = true;
    //             mVelocity = glm::vec3(0.0f);
    //             mRestartPos = boat->GetDestPos();
    //             mRestartPlanetIndex = boat->GetDestPlanet();
    //             // if (bulletGhost)
    //             // {
    //             //     btTransform t;
    //             //     t.setIdentity();
    //             //     t.setOrigin(btVector3(players[0].pos.x, players[0].pos.y, players[0].pos.z));
    //             //     bulletGhost->setWorldTransform(t);
    //             // }
    //         }
    //     } else {
    //         // if (transitionTimer <= 0.0f && !mIsDamaged && mAttackMoveLockRemaining <= 0.0f && mDodgeTimer <= 0.0f)
    //         // {
    //         //     glm::vec3 moveDelta = forward * moveForward * characterSpeed * deltaTime * dashSpeed + left * moveLeft * characterSpeed * deltaTime * dashSpeed;
    //         //     glm::vec3 desiredPos = players[0].pos + moveDelta;
    //         //     // 壁当たり：球スイープで移動経路に障害があれば移動を打ち切り
    //         //     if (bulletOk && bulletWorld && bulletWallSphere && glm::length(moveDelta) > 1e-5f)
    //         //     {
    //         //         glm::vec3 upForSweep = glm::normalize(players[0].pos - planets[players[0].planetIndex].center);
    //         //         glm::vec3 sweepFrom = players[0].pos + upForSweep * 0.4f; // 腰高で判定（地面に当たりにくくする）
    //         //         glm::vec3 sweepTo = desiredPos + upForSweep * 0.4f;
    //         //         btTransform fromBt, toBt;
    //         //         fromBt.setIdentity();
    //         //         fromBt.setOrigin(btVector3(sweepFrom.x, sweepFrom.y, sweepFrom.z));
    //         //         toBt.setIdentity();
    //         //         toBt.setOrigin(btVector3(sweepTo.x, sweepTo.y, sweepTo.z));
    //         //         btVector3 sweepFromBt(sweepFrom.x, sweepFrom.y, sweepFrom.z);
    //         //         btVector3 sweepToBt(sweepTo.x, sweepTo.y, sweepTo.z);
    //         //         btCollisionWorld::ClosestConvexResultCallback sweepCallback(sweepFromBt, sweepToBt);
    //         //         bulletWorld->convexSweepTest(bulletWallSphere, fromBt, toBt, sweepCallback);
    //         //         if (sweepCallback.hasHit())
    //         //         {
    //         //             // 壁手前で一度止め、残りを壁に沿う方向（スライド）に投影して進める
    //         //             float allowFrac = std::max(0.0f, sweepCallback.m_closestHitFraction - 0.02f);
    //         //             glm::vec3 posAfterHit = players[0].pos + moveDelta * allowFrac;
    //         //             glm::vec3 hitNormGlm(
    //         //                 sweepCallback.m_hitNormalWorld.x(),
    //         //                 sweepCallback.m_hitNormalWorld.y(),
    //         //                 sweepCallback.m_hitNormalWorld.z());
    //         //             // 阻害された移動を壁面に投影 → 壁沿いのスライドベクトル
    //         //             glm::vec3 blocked = moveDelta * (1.0f - allowFrac);
    //         //             glm::vec3 slideVec = blocked - hitNormGlm * glm::dot(blocked, hitNormGlm);
    //         //             const float slideEps = 1e-4f;
    //         //             if (glm::length(slideVec) > slideEps)
    //         //             {
    //         //                 glm::vec3 slideFrom = posAfterHit + upForSweep * 0.4f;
    //         //                 glm::vec3 slideTo = slideFrom + slideVec;
    //         //                 btTransform fromBt2, toBt2;
    //         //                 fromBt2.setIdentity();
    //         //                 fromBt2.setOrigin(btVector3(slideFrom.x, slideFrom.y, slideFrom.z));
    //         //                 toBt2.setIdentity();
    //         //                 toBt2.setOrigin(btVector3(slideTo.x, slideTo.y, slideTo.z));
    //         //                 btVector3 sFrom(slideFrom.x, slideFrom.y, slideFrom.z);
    //         //                 btVector3 sTo(slideTo.x, slideTo.y, slideTo.z);
    //         //                 btCollisionWorld::ClosestConvexResultCallback slideCallback(sFrom, sTo);
    //         //                 bulletWorld->convexSweepTest(bulletWallSphere, fromBt2, toBt2, slideCallback);
    //         //                 float slideAllow = slideCallback.hasHit()
    //         //                                     ? std::max(0.0f, slideCallback.m_closestHitFraction - 0.02f)
    //         //                                     : 1.0f;
    //         //                 desiredPos = posAfterHit + slideVec * slideAllow;
    //         //             }
    //         //             else
    //         //             {
    //         //                 desiredPos = posAfterHit;
    //         //             }
    //         //         }
    //         //     }
    //         //     players[0].pos = desiredPos;
    //         //     if (players[0].onGround && jumpPressed)
    //         //     {
    //         //         players[0].velocity += up * 5.0f;
    //         //         players[0].onGround = false;
    //         //     }
    //         // }

    //         // Bボタン：向いている方向へ回避開始
    //         const float dodgeDuration = 0.5f;
    //         const float dodgeCooldownTime = 1.0f;
    //         float dodgeStartHeight = 0.0f;
    //         glm::vec3 dodgeDir(0.0f);
    //         if (mDodgePressed && !mDodgePressedPrev && mDodgeCooldown <= 0.0f && mDodgeTimer <= 0.0f && mAttackDodgeLockRemaining <= 0.0f)
    //         {
    //             glm::vec3 dodgeFwd, dodgeLeftUnused;
    //             getForwardLeft(up, mFacingYaw, dodgeFwd, dodgeLeftUnused);
    //             dodgeDir = -dodgeFwd;
    //             mDodgeTimer = dodgeDuration;
    //             mDodgeCooldown = dodgeCooldownTime;
    //             dodgeStartHeight = glm::length(mPos - currentPlanet->GetCenter());
    //             mVelocity = glm::vec3(0.0f);
    //         }
    //         if (mDodgeTimer > 0.0f)
    //         {
    //             float dodgeSpeed = dodgeDistance / dodgeDuration;
    //             mPos += dodgeDir * dodgeSpeed * deltaTime;
    //             glm::vec3 center = currentPlanet->GetCenter();
    //             // 空中回避：直前の高さを維持して浮遊
    //             float dist = glm::length(mPos - center);
    //             if (dist > 1e-6f)
    //                 mPos = center + (mPos - center) / dist * dodgeStartHeight;
    //             mDodgeTimer -= deltaTime;
    //         }
    //         if (mDodgeCooldown > 0.0f)
    //             mDodgeCooldown -= deltaTime;

    //         // 攻撃後＋0.5秒：攻撃時の高さを維持して浮遊（空中固定）
    //         if (mAttackHeightLockRemaining > 0.0f)
    //         {
    //             glm::vec3 center = currentPlanet->GetCenter();
    //             float dist = glm::length(mPos - center);
    //             if (dist > 1e-6f)
    //                 mPos = center + (mPos - center) / dist * attackStartHeight;
    //         }
    //         // Bullet レイキャスト：足元にメッシュがあれば地形に沿わせ、穴の上なら重力で落ちる
    //         // 回避中・攻撃硬直中は浮遊のためスキップ。上昇中（ジャンプ直後）もスキップし、地上 or 落下中のみ判定
    //         glm::vec3 upForJump = glm::normalize(players[0].pos - planets[players[0].planetIndex].center);
    //         bool isRising = glm::dot(players[0].velocity, upForJump) > 0.5f;
    //         bool meshGround = false;
    //         if (mDodgeTimer <= 0.0f && mAttackHeightLockRemaining <= 0.0f && bulletOk && bulletWorld && !isRising)
    //         {
    //             glm::vec3 center = planets[players[0].planetIndex].center;
    //             glm::vec3 upDir = glm::normalize(players[0].pos - center);
    //             glm::vec3 rayFrom3 = players[0].pos + upDir * 0.1f;
    //             glm::vec3 rayTo3 = players[0].pos - upDir * 0.1f;
    //             btVector3 rayFrom(rayFrom3.x, rayFrom3.y, rayFrom3.z);
    //             btVector3 rayTo(rayTo3.x, rayTo3.y, rayTo3.z);
    //             btCollisionWorld::ClosestRayResultCallback rayCallback(rayFrom, rayTo);
    //             bulletWorld->rayTest(rayFrom, rayTo, rayCallback);
    //             if (rayCallback.hasHit())
    //             {
    //                 btVector3 hitPt = rayCallback.m_hitPointWorld;
    //                 glm::vec3 hitPos(hitPt.x(), hitPt.y(), hitPt.z());
    //                 float hitDist = glm::length(hitPos - center);
    //                 float playerDist = glm::length(players[0].pos - center);
    //                 if (playerDist - hitDist < 2.0f)
    //                 {
    //                     players[0].pos = hitPos;
    //                     players[0].onGround = true;
    //                     players[0].velocity = glm::vec3(0.0f);
    //                     meshGround = true;
    //                 }
    //             }
    //             if (!meshGround && players[0].onGround)
    //             {
    //                 players[0].onGround = false;
    //             }
    //         }
    //         // 回避中・攻撃空中固定中は重力をかけず、終了後に通常通り落下
    //         if (mDodgeTimer <= 0.0f && mAttackHeightLockRemaining <= 0.0f)
    //             // updatePlayerPhysics(players[0], deltaTime, planets, &transitionTimer, bulletOk && bulletWorld);

    //         // 落下して惑星内部にめり込んだらリスタート地点へ
    //         if (bulletOk && bulletWorld)
    //         {
    //             float dist = glm::length(players[0].pos - planets[players[0].planetIndex].center);
    //             float r = planets[players[0].planetIndex].radius;
    //             if (dist < r * 0.5f)
    //             {
    //                 players[0].pos = restartPos;
    //                 players[0].planetIndex = restartPlanetIndex;
    //                 players[0].velocity = glm::vec3(0.0f);
    //                 players[0].onGround = true;
    //             }
    //         }
    //     }
    // }

    // glm::vec3 forward, left;
    // getForwardLeft(up, mCameraYaw, forward, left);

    // // スティックを倒した方向を向く。移動ロック中は地上のみ向き固定、空中攻撃中は向き替え可
    // if ((mAttackMoveLockRemaining <= 0.0f || !mOnGround) && (std::abs(mMoveForward) > 0.01f || std::abs(mMoveLeft) > 0.01f))
    // {
    //     glm::vec3 moveDir = forward * mMoveForward + left * mMoveLeft;
    //     float len = glm::length(moveDir);
    //     if (len > 0.001f)
    //     {
    //         // 移動方向を正規化
    //         moveDir /= len;
    //         mFacingYaw = getYawFromDirection(up, moveDir) + 3.14159265f;
    //     }
    // }

    // // 攻撃
    // const float attackRange = 1.8f;
    // const float attackAngle = 0.8f;
    // bool canAttack = attackPressed && !attackPressedPrev && mAttackCooldownRemaining <= 0.0f;
    // if (canAttack)
    // {
    //     mAttackStartHeight = glm::length(mPos - currentPlanet->GetCenter());
    //     mVelocity = glm::vec3(0.0f);
    //     bool hitTarget = false;
    //     glm::vec3 attackFwd, attackLeftUnused;
    //     getForwardLeft(mUpVec, mFacingYaw, attackFwd, attackLeftUnused);

    //     for (auto& enemy_unique : enemies)
    //     {
    //         Enemy* enemy = enemy_unique.get();
    //         glm::vec3 enemyPos = enemy->GetPos();
    //         if (!enemy->GetIsAlive())
    //             continue;
            
    //         glm::vec3 toEnemy = glm::normalize(enemyPos - mPos);
    //         float dist = glm::length(enemyPos - mPos);
    //         float dot = glm::dot(-attackFwd, toEnemy);
    //         float effectiveRange = attackRange + enemy->GetRadius();

    //         bool isHit = dist <= effectiveRange && dot >= attackAngle;
    //         if (isHit)
    //         {
    //             enemy->SetIsDamaged(true);

    //             mAttackCooldownRemaining = 0.3f;
    //             mAttackIndex++;
    //             bool wasThirdHit = false;
    //             if (mAttackIndex == 3)
    //             {
    //                 wasThirdHit = true;
    //                 mAttackCooldownRemaining = 1.5f;
    //                 mAttackIndex = 0;
    //             }
    //             mAttackMoveLockRemaining = std::min(mAttackCooldownRemaining, 1.0f) + 0.5f;
    //             mAttackDodgeLockRemaining = std::max(0.0f, mAttackMoveLockRemaining - 0.5f);
    //             if (wasThirdHit && !mOnGround)
    //                 mAttackHeightLockRemaining = 0.5f;
    //             else
    //                 mAttackHeightLockRemaining = mAttackMoveLockRemaining + 0.5f;
    //             auto it = seList.find("attackSE");
    //             Mix_Chunk* attackSE = (it != seList.end()) ? it->second : nullptr;
    //             if (attackSE)
    //                 Mix_PlayChannel(-1, attackSE, 0);
    //             hitTarget = true;
    //         }
    //     }

    //     if (!hitTarget)
    //     {
    //         mAttackCooldownRemaining = 0.3f;
    //         mAttackIndex++;

    //         bool wasThirdHit = false;
    //         if (mAttackIndex == 3)
    //         {
    //             wasThirdHit = true;
    //             mAttackCooldownRemaining = 1.5f;
    //             mAttackIndex = 0;
    //         }

    //         mAttackMoveLockRemaining = std::min(mAttackCooldownRemaining, 1.0f) + 0.5f;
    //         mAttackDodgeLockRemaining = std::max(0.0f, mAttackMoveLockRemaining - 0.5f);

    //         if (wasThirdHit && !mOnGround)
    //             mAttackHeightLockRemaining = 0.7f;
    //         else
    //             mAttackHeightLockRemaining = mAttackMoveLockRemaining + 0.2f;
            
    //         auto it = seList.find("attackMissSE");
    //         Mix_Chunk* attackMissSE = (it != seList.end()) ? it->second : nullptr;
    //         if (attackMissSE)
    //             Mix_PlayChannel(-1, attackMissSE, 0);
    //     }
    // }

    // // カウンター
    // if (counterPressed && !counterKeyPressedPrev && !boatTransitionActive)
    // {
    //     glm::vec3 counterFwd, counterLeftUnused;
    //     getForwardLeft(mUp, mFacingYaw, counterFwd, counterLeftUnused);
    //     for (auto& enemy_unique : enemies)
    //     {
    //         Enemy* enemy = enemy_unique.get();
    //         glm::vec3 enemyPos = enemy->GetPos();
    //         float enemyStandByAttackTimer = enemy->GetStandByAttackTimer() ;
    //         if (!enemy->GetIsAlive())
    //             continue;
    //         if (enemyStandByAttackTimer <= 0.0f || enemyStandByAttackTimer > 0.2f)
    //             continue;
                
    //         glm::vec3 toEnemy = glm::normalize(enemyPos - mPos);
    //         float dist = glm::length(enemyPos - mPos);
    //         float dot = glm::dot(-attackFwd, toEnemy);
    //         float effectiveRange = attackRange + enemy->GetRadius();
    //         bool isHit = dist <= effectiveRange && dot >= attackAngle;
    //         if (isHit)
    //         {
    //             enemy->SetIsCountered(true);
    //             auto it = seList.find("counterSE");
    //             Mix_Chunk* counterSE = (it != seList.end()) ? it->second : nullptr;   
    //             if (counterSE)
    //                 Mix_PlayChannel(-1, counterSE, 0);
    //             break;
    //         }
    //     }
    // }

    // // 攻撃クールダウンタイム減少
    // if (mAttackCooldownRemaining >= 0.0f)
    // {
    //     mAttackCooldownRemaining -= deltaTime;
    // }

    // // 移動クールダウンタイム減少
    // if (mAttackMoveLockRemaining > 0.0f)
    // {
    //     mAttackMoveLockRemaining -= deltaTime;
    //     if (mAttackMoveLockRemaining < 0.0f)
    //         mAttackMoveLockRemaining = 0.0f;
    //     if (mAttackMoveLockRemaining <= 0.0f)
    //         mAttackIndex = 0; // 歩けるようになったらコンボリセット
    // }

    // // 回避クールダウンタイム減少
    // if (mAttackDodgeLockRemaining > 0.0f)
    // {
    //     mAttackDodgeLockRemaining -= deltaTime;
    //     if (mAttackDodgeLockRemaining < 0.0f)
    //         mAttackDodgeLockRemaining = 0.0f;
    // }

    // // 空中浮遊クールダウンタイム減少
    // if (mAttackHeightLockRemaining > 0.0f)
    // {
    //     mAttackHeightLockRemaining -= deltaTime;
    //     if (mAttackHeightLockRemaining < 0.0f)
    //         mAttackHeightLockRemaining = 0.0f;
    // }

    // // ダメージを受けた際のノックバック
    // if (mDamageTimer > 0.0f)
    // {
    //     glm::vec3 toPlayer = glm::normalize(mPos - mKnockBackFrom);
    //     mPos += toPlayer * deltaTime;
    //     mDamageTimer -= deltaTime;
    // }
    // else
    // {
    //     mIsDamaged = false;
    // }
}

// void Player::updatePlayerPhysics(PlayerState& p, float deltaTime, const std::vector<Planet>& planets,
//     float* transitionTimer, bool skipGroundSnap) {
//     glm::vec3 center = planets[p.planetIndex].center;
//     float radius = planets[p.planetIndex].radius;
//     glm::vec3 up = glm::normalize(p.pos - center);

//     if (p.onGround && !skipGroundSnap) {
//         p.pos = center + glm::normalize(p.pos - center) * radius;
//         return;
//     }
//     if (p.onGround && skipGroundSnap) {
//         return;
//     }
//     if (transitionTimer && *transitionTimer > 0.0f) {
//         *transitionTimer -= deltaTime;
//         return;
//     }
//     p.velocity -= up * 9.8f * deltaTime;
//     p.pos += p.velocity * deltaTime;

//     float distToCurrent = glm::length(p.pos - center);
//     for (size_t i = 0; i < planets.size(); i++) {
//         if (i == static_cast<size_t>(p.planetIndex)) continue;
//         float d = glm::length(p.pos - planets[i].center);
//         if (d < planets[i].radius + 2.0f && d < distToCurrent) {
//             p.planetIndex = static_cast<int>(i);
//             if (transitionTimer) *transitionTimer = 0.3f;
//             break;
//         }
//     }
//     center = planets[p.planetIndex].center;
//     radius = planets[p.planetIndex].radius;
//     float dist = glm::length(p.pos - center);
//     if (!skipGroundSnap && dist <= radius) {
//         p.onGround = true;
//         p.pos = center + glm::normalize(p.pos - center) * radius;
//         p.velocity = glm::vec3(0, 0, 0);
//     }
// }

void Player::getForwardLeft(const glm::vec3& up, float cameraYaw, glm::vec3& outForward, glm::vec3& outLeft) {
    glm::vec3 worldLeft = glm::normalize(glm::cross(up, glm::vec3(0, 0, 1)));
    if (glm::length(worldLeft) < 0.01f)
        worldLeft = glm::normalize(glm::cross(up, glm::vec3(1, 0, 0)));
    outForward = glm::normalize(glm::cross(worldLeft, up) * std::cos(cameraYaw) - std::sin(cameraYaw) * worldLeft);
    outLeft = glm::normalize(glm::cross(up, outForward));
}

float Player::getYawFromDirection(const glm::vec3& up, const glm::vec3& dir) {
    glm::vec3 worldLeft = glm::normalize(glm::cross(up, glm::vec3(0, 0, 1)));
    if (glm::length(worldLeft) < 0.01f)
        worldLeft = glm::normalize(glm::cross(up, glm::vec3(1, 0, 0)));
    glm::vec3 right = glm::cross(worldLeft, up);
    return std::atan2(-glm::dot(dir, worldLeft), glm::dot(dir, right));
}

// glm::mat4 Player::getPlayerView(const PlayerState& p, float cameraDistance, const std::vector<Planet>& planets) {
//     glm::vec3 center = planets[p.planetIndex].center;
//     glm::vec3 up = glm::normalize(p.pos - center);
//     glm::vec3 fwd, left;
//     getForwardLeft(up, p.cameraYaw, fwd, left);
//     glm::vec3 back = glm::normalize(-fwd);
//     glm::vec3 cameraDir = glm::normalize(std::cos(p.cameraPitch) * back + std::sin(p.cameraPitch) * up);
//     glm::vec3 cameraPos = p.pos - cameraDir * cameraDistance;
//     return glm::lookAt(cameraPos, p.pos, up);
// }

void Player::getPlayerFallbackTriangle(std::vector<float>& outVertices) {
    outVertices.clear();
    // 描画する三角形の3頂点の座標
    float v[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };
    for (float x : v) outVertices.push_back(x);
}