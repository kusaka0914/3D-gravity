#include "Player.h"
#include "Boat.h"
#include "Game.h"
#include <cmath>

Player::Player(Game* game)
    : Actor(game)
    , mPos(0.0f)
    , mKnockBackFrom(0.0f)
    , mRestartPos(0.0f)
    , mPlanetIndex(0)
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
    GetGame()->AddPlayer(this);
}

Player::~Player()
{
    GetGame()->RemovePlayer(this);
}

void Player::UpdateActor(float deltaTime)
{
    int currentStageNum = GetGame()->GetCurrentStageNum();
    Stage* currentStage = GetGame()->GetStages()[currentStageNum].get();
    Planet* currentPlanet = currentStage->GetPlanets()[mCurrentPlanet].get();
    vector<std::unique_ptr<Boat>> boats = currentPlanet->GetBoats();
    for (auto boat_unique : boats) {
        Boat* boat = boat_unique.get();
        bool active = boat->GetIsActive();
        if (active) {
            glm::vec3 boatPos = boat->GetPos();
            glm::vec3 boatUpVec = boat->GetUpVec();
            players[0].pos = boatPos + boatUpVec * playerHeightAboveBoat;
        }
    }
}

void updatePlayerPhysics(PlayerState& p, float deltaTime, const std::vector<Planet>& planets,
    float* transitionTimer, bool skipGroundSnap) {
    glm::vec3 center = planets[p.planetIndex].center;
    float radius = planets[p.planetIndex].radius;
    glm::vec3 up = glm::normalize(p.pos - center);

    if (p.onGround && !skipGroundSnap) {
        p.pos = center + glm::normalize(p.pos - center) * radius;
        return;
    }
    if (p.onGround && skipGroundSnap) {
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
    if (!skipGroundSnap && dist <= radius) {
        p.onGround = true;
        p.pos = center + glm::normalize(p.pos - center) * radius;
        p.velocity = glm::vec3(0, 0, 0);
    }
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