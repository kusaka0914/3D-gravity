#include "player.h"
#include <cmath>

void getPlayerFallbackTriangle(std::vector<float>& outVertices) {
    outVertices.clear();
    // 描画する三角形の3頂点の座標
    float v[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };
    for (float x : v) outVertices.push_back(x);
}

void getForwardLeft(const glm::vec3& up, float cameraYaw, glm::vec3& outForward, glm::vec3& outLeft) {
    glm::vec3 worldLeft = glm::normalize(glm::cross(up, glm::vec3(0, 0, 1)));
    if (glm::length(worldLeft) < 0.01f)
        worldLeft = glm::normalize(glm::cross(up, glm::vec3(1, 0, 0)));
    outForward = glm::normalize(glm::cross(worldLeft, up) * std::cos(cameraYaw) - std::sin(cameraYaw) * worldLeft);
    outLeft = glm::normalize(glm::cross(up, outForward));
}

float getYawFromDirection(const glm::vec3& up, const glm::vec3& dir) {
    glm::vec3 worldLeft = glm::normalize(glm::cross(up, glm::vec3(0, 0, 1)));
    if (glm::length(worldLeft) < 0.01f)
        worldLeft = glm::normalize(glm::cross(up, glm::vec3(1, 0, 0)));
    glm::vec3 right = glm::cross(worldLeft, up);
    return std::atan2(-glm::dot(dir, worldLeft), glm::dot(dir, right));
}

void updatePlayerPhysics(PlayerState& p, float deltaTime, const std::vector<Planet>& planets,
    float* transitionTimer) {
    glm::vec3 center = planets[p.planetIndex].center;
    float radius = planets[p.planetIndex].radius;
    glm::vec3 up = glm::normalize(p.pos - center);

    if (p.onGround) {
        p.pos = center + glm::normalize(p.pos - center) * radius;
        return;
    }
    if (transitionTimer && *transitionTimer > 0.0f) {
        *transitionTimer -= deltaTime;
        return;
    }
    p.velocity -= up * 9.8f * deltaTime;
    p.pos += p.velocity * deltaTime;

    float distToCurrent = glm::length(p.pos - center);
    for (size_t i = 0; i < planets.size(); i++) {
        if (i == static_cast<size_t>(p.planetIndex)) continue;
        float d = glm::length(p.pos - planets[i].center);
        if (d < planets[i].radius + 2.0f && d < distToCurrent) {
            p.planetIndex = static_cast<int>(i);
            if (transitionTimer) *transitionTimer = 0.3f;
            break;
        }
    }
    center = planets[p.planetIndex].center;
    radius = planets[p.planetIndex].radius;
    float dist = glm::length(p.pos - center);
    if (dist <= radius) {
        p.onGround = true;
        p.pos = center + glm::normalize(p.pos - center) * radius;
        p.velocity = glm::vec3(0, 0, 0);
    }
}

glm::mat4 getPlayerView(const PlayerState& p, float cameraDistance, const std::vector<Planet>& planets) {
    glm::vec3 center = planets[p.planetIndex].center;
    glm::vec3 up = glm::normalize(p.pos - center);
    glm::vec3 fwd, left;
    getForwardLeft(up, p.cameraYaw, fwd, left);
    glm::vec3 back = glm::normalize(-fwd);
    glm::vec3 cameraDir = glm::normalize(std::cos(p.cameraPitch) * back + std::sin(p.cameraPitch) * up);
    glm::vec3 cameraPos = p.pos - cameraDir * cameraDistance;
    return glm::lookAt(cameraPos, p.pos, up);
}
