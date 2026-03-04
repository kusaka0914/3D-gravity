#ifndef PLAYER_H
#define PLAYER_H

#include "planet.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

struct PlayerState {
    glm::vec3 pos{0};
    int planetIndex = 0;
    float cameraYaw = 0.0f;
    float cameraPitch = 0.4f;
    float facingYaw = 0.0f;
    glm::vec3 velocity{0, 0, 0};
    bool onGround = true;
    float attack = 0.0f;
    float hp = 0.0f;
    bool isDamaged = false;
    bool isDamagePrev = false;
    float damageTimer = 0.0f;
    float attackCooldownRemaining = 0.0f;
    float attackMoveLockRemaining = 0.0f;   // 攻撃後の移動禁止時間
    float attackDodgeLockRemaining = 0.0f;  // 攻撃後の回避禁止時間（移動より0.5秒短い）
    int attackIndex = 0;
};

void getForwardLeft(const glm::vec3& up, float cameraYaw, glm::vec3& outForward, glm::vec3& outLeft);

/** 接平面内の方向 dir から getForwardLeft と対応する yaw を返す（スティック方向を向く用） */
float getYawFromDirection(const glm::vec3& up, const glm::vec3& dir);

void updatePlayerPhysics(PlayerState& p, float deltaTime, const std::vector<Planet>& planets,
    float* transitionTimer = nullptr, bool skipGroundSnap = false);

glm::mat4 getPlayerView(const PlayerState& p, float cameraDistance, const std::vector<Planet>& planets);

void getPlayerFallbackTriangle(std::vector<float>& outVertices);

#endif
