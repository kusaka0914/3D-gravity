#include "Renderer3D.h"
#include "gfx/Shader3D.h"
#include "actor/Player.h"
#include "Stage.h"
#include "actor/Enemy.h"
#include "actor/Boat.h"
#include "actor/Planet.h"
#include "actor/BoatParts.h"
#include "actor/Key.h"
#include "actor/Crystal.h"
#include "actor/Star.h"
#include "VertexArray.h"
#include "Game.h"
#include "actor/NPC.h"
#include "utils/MathUtils.h"
#include "actor/Platform.h"
#include "system/SceneSystem.h"
#include "system/CameraSystem.h"
#include "thirdParty/stb_image.h"
#include "component/FocusComponent.h"
#include "system/MeshLoadSystem.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>
#include <memory>

Renderer3D::Renderer3D(Game* game)
    : Renderer(game)
{
    Initialize();
}

Renderer3D::~Renderer3D() = default;

void Renderer3D::Initialize() {
    mShader3DUnique = std::make_unique<Shader3D>();
    mShader3D = mShader3DUnique.get();

    if (!mShader3D->GetShaderProgram())
    {
        glfwTerminate();
        return;
    }

    RegisterTexture("../assets/textures/guard.png", "guard");

    glGenVertexArrays(1, &mAttackRangeVAO);
    glGenBuffers(1, &mAttackRangeVBO);

    glBindVertexArray(mAttackRangeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mAttackRangeVBO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec3),
        reinterpret_cast<void*>(0)
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer3D::Draw() {
    bool isTitle = mGame->GetSceneSystem()->IsTitle();
    if (isTitle) return;

    GLFWwindow* window = mGame->GetWindow();
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glUseProgram(mShader3D->GetShaderProgram());

    std::vector<glm::mat4> views = GetViews();

    bool isPlayer2Joined = mGame->GetIsPlayer2Joined();
    if (!isPlayer2Joined)
    {
        glViewport(0, 0, fbWidth, fbHeight);
        float aspect = static_cast<float>(fbWidth) / static_cast<float>(fbHeight);
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
        DrawScene(views[0], proj);
        return;
    }

    float halfW = fbWidth * 0.5f;
    float aspectHalf = halfW / static_cast<float>(fbHeight);
    glm::mat4 projHalf = glm::perspective(glm::radians(45.0f), aspectHalf, 0.1f, 100.0f);
    glViewport(0, 0, static_cast<GLsizei>(halfW), fbHeight);
    DrawScene(views[0], projHalf);
    glViewport(static_cast<GLsizei>(halfW), 0, static_cast<GLsizei>(halfW), fbHeight);
    DrawScene(views[1], projHalf);
}

void Renderer3D::DrawScene(const glm::mat4 &viewMat, const glm::mat4 &projMat) {
    GLint locModel = mShader3D->GetLocModel();
    GLint locView = mShader3D->GetLocView();
    GLint locProj = mShader3D->GetLocProj();
    GLint locObjectColor = mShader3D->GetLocObjectColor();
    GLint locUseTexture = mShader3D->GetLocUseTexture();
    GLint locDiffuseTexture = mShader3D->GetLocDiffuseTexture();
    GLint locLightPos = mShader3D->GetLocLightPos();
    GLint locLightColor = mShader3D->GetLocLightColor();
    GLint locViewPos = mShader3D->GetLocViewPos();
    GLint locAmbientStrength = mShader3D->GetLocAmbientStrength();
    GLint locToonLevels = mShader3D->GetLocToonLevels();
    GLint locToonStrength = mShader3D->GetLocToonStrength();
    GLint locRimStrength = mShader3D->GetLocRimStrength();
    GLint locRimPower = mShader3D->GetLocRimPower();

    glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(viewMat));
    glUniformMatrix4fv(locProj, 1, GL_FALSE, glm::value_ptr(projMat));
    std::vector<Player*> players = mGame->GetPlayers();
    glm::vec3 cameraPos = mGame->GetCameraSystem()->GetCameraPos();
    glUniform3f(locLightPos, cameraPos.x,cameraPos.y, cameraPos.z);
    glUniform3f(locLightColor, 0.5f, 0.5f, 0.5f);
    glUniform3f(locViewPos, cameraPos.x, cameraPos.y, cameraPos.z);

    glUniform1f(locAmbientStrength, 0.8f);
    glUniform1f(locToonLevels, 5.0f);
    glUniform1f(locToonStrength, 0.45f);

    glUniform1f(locRimStrength, 0.20f);
    glUniform1f(locRimPower, 2.5f);

    Stage* currentStage = mGame->GetCurrentStage();
    if (!currentStage) return;

    std::vector<Planet*> planets = currentStage->GetPlanets();

    if (!planets.empty()) {
        for (auto planet : planets)
        {
            glm::mat4 planetModel = glm::translate(glm::mat4(1.0f), planet->GetPos()) * glm::scale(glm::mat4(1.0f), glm::vec3(planet->GetScale()));
            glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(planetModel));
            glUniform4f(locObjectColor, planet->GetColor().x, planet->GetColor().y, planet->GetColor().z, planet->GetColor().w);
            
            for (auto m : *planet->GetMeshes())
            {
                glBindVertexArray(m.VAO);
                if (m.textureID != 0)
                {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, m.textureID);
                    glUniform1i(locDiffuseTexture, 0);
                    glUniform1i(locUseTexture, 1);
                }
                else
                {
                    glUniform1i(locUseTexture, 0);
                }
                glDrawElements(GL_TRIANGLES, m.indexCount, GL_UNSIGNED_INT, 0);
            }
            glUniform1i(locUseTexture, 0);
        }
    }

    glm::vec3 playerScale = glm::vec3(0.25f);
    if (players[0]->GetIsActive()) {
        DrawCharacter(players[0]->GetPos(), playerScale, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), players[0]->GetUpVec(), players[0]->GetFacingYaw(), players[0]->GetMeshes());
    }

    bool isPlayer2Joined = mGame->GetIsPlayer2Joined();
    if (isPlayer2Joined)
    {
        DrawCharacter(players[1]->GetPos(), playerScale, glm::vec4(1.0f, 0.5f, 0.0f, 1.0f), players[1]->GetUpVec(), players[1]->GetFacingYaw(),  players[1]->GetMeshes());
    }

    if (players[0]->GetAttackMotionTimer() >= 0.0f) {
        DrawAttackRange(players[0]);
    }

    Planet* currentPlanet = players[0]->GetCurrentPlanet();
    if (!currentPlanet) return;

    std::vector<Enemy*> enemies = currentPlanet->GetEnemies();
    if (!enemies.empty()) {
        for (auto enemy : enemies)
        {
            if (enemy->GetIsDead())
                continue;
            
            DrawCharacter(enemy->GetPos(), enemy->GetScale(), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), enemy->GetUpVec(), enemy->GetFacingYaw(), enemy->GetMeshes());
            DrawGuard(viewMat, enemy);
        }
    }

    Key* key = currentPlanet->GetKey();
    if (key) {
        if (key->GetIsActive())
        {
            glm::vec3 keyScale = glm::vec3(2.0f);
            const glm::vec4 keyColor(0.85f, 0.65f, 0.13f, 1.0f);
            DrawCharacter(key->GetPos(), keyScale, keyColor, key->GetUpVec(), 0.0f, key->GetMeshes(), &keyColor);
        }
    }

    std::vector<NPC*> NPCs = currentPlanet->GetNPCs();
    if (!NPCs.empty()) {
        for (auto NPC : NPCs) {
            if (NPC->GetIsActive())
            {
                glm::vec3 NPCScale = glm::vec3(0.25f);
                DrawCharacter(NPC->GetPos(), NPCScale, glm::vec4(0.4f, 0.25f, 0.1f, 1.0f), NPC->GetUpVec(), NPC->GetFacingYaw(), NPC->GetMeshes());
            }
        }
    }

    std::vector<Boat*> boats = currentPlanet->GetBoats();
    if (!boats.empty()) {
        for (auto boat : boats) {
            if (boat->GetIsActive())
            {
                glm::vec3 boatScale = glm::vec3(0.8f);
                DrawCharacter(boat->GetPos(), boatScale, glm::vec4(0.4f, 0.25f, 0.1f, 1.0f), boat->GetUpVec(), boat->GetFacingYaw(), boat->GetMeshes());
            }
        }
    }

    std::vector<BoatParts*> boatParts = currentPlanet->GetBoatParts();
    if (!boatParts.empty()) {
        for (auto parts : boatParts) { 
            if (parts->GetIsActive()) {
                glm::vec3 boatPartsScale = glm::vec3(1.0f);
                DrawCharacter(parts->GetPos(), boatPartsScale, glm::vec4(0.4f, 0.25f, 0.1f, 1.0f), parts->GetUpVec(), 0.0f, parts->GetMeshes());
            }
        }
    }

    Star* star = currentPlanet->GetStar();
    if (star) {
        if (star->GetIsActive())
        {
            glm::vec4 starColor(1.0f, 0.9f, 0.2f, 1.0f);
            glm::vec3 starScale = glm::vec3(0.3f);
            DrawCharacter(star->GetPos(), starScale, starColor, star->GetUpVec(), 0.0f, star->GetMeshes());
        }
    }

    std::vector<Crystal*> crystals = currentPlanet->GetCrystals();
    if (!crystals.empty()) {
        for (auto crystal : crystals) { 
            if (crystal->GetIsActive()) {
                DrawCharacter(crystal->GetPos(), crystal->GetScale(), glm::vec4(0.4f, 0.25f, 0.1f, 1.0f), crystal->GetUpVec(), 0.0f, crystal->GetMeshes());
            }
        }
    }

    std::vector<Platform*> platforms = currentPlanet->GetPlatforms();
    if (!platforms.empty()) {
        for (auto platform : platforms) { 
            if (platform->GetIsActive()) {
                DrawCharacter(platform->GetPos(), platform->GetScale(), glm::vec4(0.4f, 0.25f, 0.1f, 1.0f), platform->GetUpVec(), 0.0f, platform->GetMeshes());
            }
        }
    }
};

void Renderer3D::DrawCharacter(const glm::vec3 &pos, glm::vec3 scale, const glm::vec4 &fallbackColor,
    const glm::vec3 &up, float yaw, const std::vector<struct LoadedMesh> *Meshes,
    const glm::vec4 *colorOverride) 
{
    GLint locModel = mShader3D->GetLocModel();
    GLint locObjectColor = mShader3D->GetLocObjectColor();
    GLint locUseTexture = mShader3D->GetLocUseTexture();
    GLint locDiffuseTexture = mShader3D->GetLocDiffuseTexture();

    glm::vec3 upN = glm::normalize(up);
    glm::vec3 worldLeft = glm::cross(upN, glm::vec3(0, 0, 1));
    if (glm::length(worldLeft) < 0.01f){
        worldLeft = glm::normalize(glm::cross(upN, glm::vec3(0, 1, 0)));
    }
    else 
        worldLeft = glm::normalize(worldLeft);
    glm::vec3 fwd = glm::normalize(glm::cross(worldLeft, upN) * std::cos(yaw) - std::sin(yaw) * worldLeft);
    glm::vec3 left = glm::normalize(glm::cross(upN, fwd));
    glm::vec3 right = -left;
    glm::mat4 orient = glm::mat4(1.0f);
    orient[0] = glm::vec4(-fwd, 0.0f);
    orient[1] = glm::vec4(up, 0.0f);
    orient[2] = glm::vec4(right, 0.0f);
    orient[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), pos) * orient * glm::scale(glm::mat4(1.0f), glm::vec3(scale));
    glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));

    if (!Meshes->empty())
    {
        for (const LoadedMesh &m : *Meshes)
        {
            glBindVertexArray(m.VAO);
            if (m.textureID != 0)
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m.textureID);
                glUniform1i(locDiffuseTexture, 0);
                glUniform1i(locUseTexture, 1);
            }
            else
            {
                glUniform1i(locUseTexture, 0);
            }
            if (colorOverride)
            {
                glUniform4f(locObjectColor, colorOverride->x, colorOverride->y, colorOverride->z, colorOverride->w);
            }
            else
            {
                glUniform4f(locObjectColor, m.diffuseColor[0], m.diffuseColor[1], m.diffuseColor[2], 1.0f);
            }
            glDrawElements(GL_TRIANGLES, m.indexCount, GL_UNSIGNED_INT, 0);
        }
        glUniform1i(locUseTexture, 0);
        return;
    }
}

void Renderer3D::DrawGuard(glm::mat4 viewMat, Enemy* enemy) {
    GLint locModel = mShader3D->GetLocModel();
    GLint locObjectColor = mShader3D->GetLocObjectColor();
    GLint locUseTexture = mShader3D->GetLocUseTexture();
    GLint locDiffuseTexture = mShader3D->GetLocDiffuseTexture();

    GLuint texId = mTextures["guard"];
    glm::vec3 enemyPos = enemy->GetPos();
    glm::vec3 enemyUp = enemy->GetUpVec();
    int enemyBreakCount = enemy->GetBreakCount();
    float enemyRadius = enemy->GetRadius();

    glm::vec3 camPos(glm::inverse(viewMat)[3]);
    glm::vec3 quadCenter = enemyPos + enemyUp * 0.8f;
    glm::vec3 forward = glm::normalize(camPos - quadCenter);
    glm::vec3 right = glm::normalize(glm::cross(enemyUp, forward));
    if (glm::length(right) < 0.01f)
        right = glm::normalize(glm::cross(enemyUp, glm::vec3(0, 0, 1)));
    glm::vec3 upQuad = glm::cross(forward, right);
    
    const float enemyLabelWidth = 0.5f;
    const float enemyLabelHeight = 0.5f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texId);
    glUniform1i(locUseTexture, 1);
    mVertexArrays.at("text")->SetActive();
    glm::mat4 billboard(1.0f);
    billboard[0] = glm::vec4(right * enemyLabelWidth, 0.0f);
    billboard[1] = glm::vec4(-upQuad * enemyLabelHeight, 0.0f);
    billboard[2] = glm::vec4(forward, 0.0f);
    
    int breakCount = enemy->GetBreakCount();
    if (breakCount != 0) {
        float rightMargin = 0;
        for (int i = 0; i < enemyBreakCount; i++) {
            float rightMargin = (i - (enemyBreakCount - 1) * 0.5f) * 0.4f;
            glm::vec3 drawPos = enemyPos + enemyUp * enemyRadius * 0.8f + right * rightMargin;
            billboard[3] = glm::vec4(drawPos, 1.0f);
            glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(billboard));
            glDrawArrays(GL_TRIANGLES, 0, 6);           
        }

        glUniform1i(locUseTexture, 0);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(locUseTexture, 0);

    mVertexArrays.at("hpBar")->SetActive();
    float hp = enemy->GetHp();
    float maxHp = enemy->GetMaxHp();
    float hpWidth = hp / maxHp;
    billboard[0] = glm::vec4(right * hpWidth, 0.0f);
    billboard[1] = glm::vec4(-upQuad * 0.1f, 0.0f);
    billboard[2] = glm::vec4(forward, 0.0f);
    glm::vec3 drawPos = enemyPos + enemyUp * enemyRadius * 1.5f - right * 0.5f;
    billboard[3] = glm::vec4(drawPos, 1.0f);
    glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(billboard));
    std::vector<GLfloat> color{0.0f, 1.0f, 0.0f, 1.0f};
    glUniform4fv(locObjectColor, 1, color.data());
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

std::vector<glm::mat4> Renderer3D::GetViews() {
    std::vector<glm::mat4> views;

    bool isOpening = mGame->GetSceneSystem()->IsOpening();
    if (isOpening) {
        bool isTalkWithMother = mGame->GetSceneSystem()->IsTalkWithMother();
        bool isTalkWithDoctor = mGame->GetSceneSystem()->IsTalkWithDoctor();

        if (isTalkWithMother) {
            glm::mat4 view = glm::lookAt(glm::vec3(-2.0f, 4.0f, -2.0f), glm::vec3(4.0f, 2.0f, 4.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            views.emplace_back(view);
        } else if (isTalkWithDoctor) {
            glm::mat4 view = glm::lookAt(glm::vec3(3.0f, 4.0f, 1.0f), glm::vec3(-4.0f, 2.0f, -4.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            views.emplace_back(view);
        }

        if(!views.empty())
            return views;
    }

    std::vector<Player*> players = mGame->GetPlayers();
    Planet* currentPlanet = players[0]->GetCurrentPlanet();

    std::vector<Boat*> boats = currentPlanet->GetBoats();
    if (!boats.empty()) {
        for (auto boat : boats) {
            if (boat->GetFocusComponent()->GetFocusTimer() < 0.0f)
                continue;
                
            glm::mat4 view = mGame->GetCameraSystem()->GetFocusView(boat);
            views.emplace_back(view);
        }
    }

    Key* key = currentPlanet->GetKey();
    if (key) {
        if (key->GetFocusComponent()->GetFocusTimer() >= 0.0f) {
            glm::mat4 view = mGame->GetCameraSystem()->GetFocusView(key);
            views.emplace_back(view);
        }
    }
    
    bool isStageClear = mGame->GetSceneSystem()->IsStageClear();
    if (isStageClear) {
        glm::mat4 view = mGame->GetCameraSystem()->GetPlayerView(4.0f, true);
        views.emplace_back(view);
    }

    if (views.empty()) {
        glm::mat4 view = mGame->GetCameraSystem()->GetPlayerView(10.0f);
        views.emplace_back(view);
    }    

    bool isPlayer2Joined = mGame->GetIsPlayer2Joined();
    if (isPlayer2Joined) {
        glm::mat4 view = mGame->GetCameraSystem()->GetPlayerView(4.0f, true);
        views.emplace_back(view);
    }

    return views;
}

void Renderer3D::DrawAttackRange(Player* player) {
    if (!player) return;

    constexpr int segments = 48;

    const float radius = player->GetAttackRange();
    const float attackAngleDeg = player->GetAttackAngle();

    if (radius <= 0.0f || attackAngleDeg <= 0.0f) return;

    const glm::vec3 center = player->GetPos();
    const glm::vec3 upN = glm::normalize(player->GetUpVec());
    const float yaw = player->GetFacingYaw();

    glm::vec3 worldLeft = glm::cross(upN, glm::vec3(0.0f, 0.0f, 1.0f));
    if (glm::length(worldLeft) < 0.01f) {
        worldLeft = glm::cross(upN, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    worldLeft = glm::normalize(worldLeft);

    glm::vec3 forward = glm::normalize(
        glm::cross(worldLeft, upN) * std::cos(yaw)
        - worldLeft * std::sin(yaw)
    );

    glm::vec3 side = glm::normalize(glm::cross(upN, forward));

    const float halfAngle = attackAngleDeg * 0.5f;
    const float yOffset = 0.06f;

    std::vector<glm::vec3> fillVertices;
    fillVertices.reserve(segments + 2);

    fillVertices.emplace_back(center + upN * yOffset);

    for (int i = 0; i <= segments; i++) {
        const float t = static_cast<float>(i) / static_cast<float>(segments);
        const float angle = -halfAngle + halfAngle * 2.0f * t;

        glm::vec3 dir = glm::normalize(
            -forward * std::cos(angle)
            + side * std::sin(angle)
        );

        fillVertices.emplace_back(center + dir * radius + upN * yOffset);
    }

    DrawAttackRangeVertices(
        fillVertices,
        GL_TRIANGLE_FAN,
        glm::vec4(1.0f, 0.1f, 0.1f, 0.18f)
    );

    const float thickness = 0.08f;
    const float innerRadius = std::max(0.0f, radius - thickness * 0.5f);
    const float outerRadius = radius + thickness * 0.5f;

    std::vector<glm::vec3> arcVertices;
    arcVertices.reserve((segments + 1) * 2);

    for (int i = 0; i <= segments; i++) {
        const float t = static_cast<float>(i) / static_cast<float>(segments);
        const float angle = -halfAngle + halfAngle * 2.0f * t;

        glm::vec3 dir = glm::normalize(
            -forward * std::cos(angle)
            + side * std::sin(angle)
        );

        arcVertices.emplace_back(center + dir * outerRadius + upN * yOffset);
        arcVertices.emplace_back(center + dir * innerRadius + upN * yOffset);
    }

    DrawAttackRangeVertices(
        arcVertices,
        GL_TRIANGLE_STRIP,
        glm::vec4(1.0f, 0.1f, 0.1f, 0.75f)
    );
}

void Renderer3D::DrawAttackRangeVertices(
    const std::vector<glm::vec3>& vertices,
    GLenum drawMode,
    const glm::vec4& color
) {
    if (vertices.empty()) return;

    glUniformMatrix4fv(
        mShader3D->GetLocModel(),
        1,
        GL_FALSE,
        glm::value_ptr(glm::mat4(1.0f))
    );

    glUniform1i(mShader3D->GetLocUseTexture(), 0);
    glUniform4f(
        mShader3D->GetLocObjectColor(),
        color.r,
        color.g,
        color.b,
        color.a
    );

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glBindVertexArray(mAttackRangeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mAttackRangeVBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(glm::vec3),
        vertices.data(),
        GL_DYNAMIC_DRAW
    );

    glDrawArrays(drawMode, 0, static_cast<GLsizei>(vertices.size()));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}