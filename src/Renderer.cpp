#include "Renderer.h"
#include "Shader.h"
#include "Player.h"
#include "Stage.h"
#include "Enemy.h"
#include "Boat.h"
#include "BoatParts.h"
#include "Key.h"
#include "Crystal.h"
#include "Star.h"
#include "VertexArray.h"
#include "Game.h"
#include "NPC.h"
#include "UIState.h"
#include "stb_image.h"
#include "FocusComponent.h"
#include "GameProgressState.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>
#include <memory>

Renderer::Renderer(Game* game)
    : mGame(game)
    , mShader(game->GetShader())
    , mFont(game->GetFont())
    , mVertexArrays(game->GetVertexArrays())
{
    Initialize();
}

void Renderer::Initialize() {
    AddImgInfo("../assets/textures/guard.png", "guard");
}

void Renderer::Draw() {
    bool isTitle = GetGame()->GetGameProgressState()->GetSceneState() == GameProgressState::SceneState::Title;
    if (isTitle) 
        return;

    GLFWwindow* window = mGame->GetWindow();
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(mShader->GetShaderProgram());

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

void Renderer::DrawScene(const glm::mat4 &viewMat, const glm::mat4 &projMat) {
    GLint locModel = mShader->GetLocModel();
    GLint locView = mShader->GetLocView();
    GLint locProj = mShader->GetLocProj();
    GLint locObjectColor = mShader->GetLocObjectColor();
    GLint locUseTexture = mShader->GetLocUseTexture();
    GLint locDiffuseTexture = mShader->GetLocDiffuseTexture();

    glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(viewMat));
    glUniformMatrix4fv(locProj, 1, GL_FALSE, glm::value_ptr(projMat));

    Stage* currentStage = mGame->GetCurrentStage();
    if (!currentStage)
        return;

    std::vector<Planet*> planets = currentStage->GetPlanets();

    if (!planets.empty()) {
        // 惑星描画
        for (auto planet : planets)
        {
            glm::mat4 planetModel = glm::translate(glm::mat4(1.0f), planet->GetCenter()) * glm::scale(glm::mat4(1.0f), glm::vec3(planet->GetRadius()));
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

    std::vector<Player*> players = mGame->GetPlayers();
    const float playerScale = 0.25f;
    // 1Pの描画
    if (players[0]->GetIsActive()) {
        DrawCharacter(players[0]->GetPos(), playerScale, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), players[0]->GetUpVec(), players[0]->GetFacingYaw(), players[0]->GetMeshes());
    }

    // 2Pの描画
    bool isPlayer2Joined = mGame->GetIsPlayer2Joined();
    if (isPlayer2Joined)
    {
        DrawCharacter(players[1]->GetPos(), playerScale, glm::vec4(1.0f, 0.5f, 0.0f, 1.0f), players[1]->GetUpVec(), players[1]->GetFacingYaw(),  players[1]->GetMeshes());
    }

    if (players[0]->GetAttackMotionTimer() >= 0.0f) {
        glm::vec3 up = players[0]->GetUpVec();
        float yaw = players[0]->GetFacingYaw();
                    glm::vec3 upN = glm::normalize(up);
        glm::vec3 worldLeft = glm::normalize(glm::cross(upN, glm::vec3(0, 0, 1)));
        if (glm::length(worldLeft) < 0.01f)
            worldLeft = glm::normalize(glm::cross(upN, glm::vec3(1, 0, 0)));
        glm::vec3 fwd = glm::normalize(glm::cross(worldLeft, upN) * std::cos(yaw) - std::sin(yaw) * worldLeft);
        glm::vec3 left = glm::normalize(glm::cross(upN, fwd));
        glm::vec3 right = -left;
        glm::mat4 orient = glm::mat4(1.0f);
        orient[0] = glm::vec4(-fwd, 0.0f);
        orient[1] = glm::vec4(up, 0.0f);
        orient[2] = glm::vec4(right, 0.0f);
        orient[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        glm::mat4 model = glm::translate(glm::mat4(1.0), players[0]->GetPos() + -fwd * players[0]->GetAttackRange() /2.0f + upN * 0.1f) * orient * glm::scale(glm::mat4(1.0f), glm::vec3(players[0]->GetAttackRange(), 0.1f, 1.0f));
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        mVertexArrays.at("text")->SetActive();
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    Planet* currentPlanet = players[0]->GetCurrentPlanet();
    if (!currentPlanet)
        return;

    std::vector<Enemy*> enemies = currentPlanet->GetEnemies();
    // 敵描画
    if (!enemies.empty()) {
        for (auto enemy : enemies)
        {
            if (!enemy->GetIsAlive())
                continue;
            
            glm::vec3 enemyUp = enemy->GetUpVec();
            glm::vec3 toPlayer = glm::normalize(players[0]->GetPos() - enemy->GetPos());
            float enemyFacingYaw = players[0]->getYawFromDirection(enemyUp, toPlayer) + 3.14159265f;

            DrawCharacter(enemy->GetPos(), enemy->GetScale(), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), enemyUp, enemyFacingYaw, enemy->GetMeshes());

            int breakCount = enemy->GetBreakCount();
            if (breakCount == 0)
                continue;

            DrawGuard(viewMat, enemy);
        }
    }

    // 鍵描画
    Key* key = currentPlanet->GetKey();
    if (key) {
        if (key->GetIsActive())
        {
            const float keyScale = 2.0f;
            const glm::vec4 keyColor(0.85f, 0.65f, 0.13f, 1.0f); // 金色
            DrawCharacter(key->GetPos(), keyScale, keyColor, key->GetUpVec(), 0.0f, key->GetMeshes(), &keyColor);
        }
    }

    std::vector<NPC*> NPCs = currentPlanet->GetNPCs();
    // NPC描画
    if (!NPCs.empty()) {
        for (auto NPC : NPCs) {
            if (NPC->GetIsActive())
            {
                const float NPCScale = 0.25f;
                DrawCharacter(NPC->GetPos(), NPCScale, glm::vec4(0.4f, 0.25f, 0.1f, 1.0f), NPC->GetUpVec(), NPC->GetFacingYaw(), NPC->GetMeshes());
            }
        }
    }

    std::vector<Boat*> boats = currentPlanet->GetBoats();
    // ボート描画
    if (!boats.empty()) {
        for (auto boat : boats) {
            if (boat->GetIsActive())
            {
                const float boatScale = 0.8f;
                DrawCharacter(boat->GetPos(), boatScale, glm::vec4(0.4f, 0.25f, 0.1f, 1.0f), boat->GetUpVec(), 0.0f, boat->GetMeshes());
            }
        }
    }

    // ボートのかけら描画
    std::vector<BoatParts*> boatParts = currentPlanet->GetBoatParts();
    if (!boatParts.empty()) {
        for (auto parts : boatParts) { 
            if (parts->GetIsActive()) {
                const float boatPartsScale = 1.0f;
                DrawCharacter(parts->GetPos(), boatPartsScale, glm::vec4(0.4f, 0.25f, 0.1f, 1.0f), parts->GetUpVec(), 0.0f, parts->GetMeshes());
            }
        }
    }

    // スター描画
    Star* star = currentPlanet->GetStar();
    if (star) {
        if (star->GetIsActive())
        {
            glm::vec4 starColor(1.0f, 0.9f, 0.2f, 1.0f);
            const float starScale = 0.3f;
            DrawCharacter(star->GetPos(), starScale, starColor, star->GetUpVec(), 0.0f, star->GetMeshes());
        }
    }

    // クリスタル描画
    std::vector<Crystal*> crystals = currentPlanet->GetCrystals();
    if (!crystals.empty()) {
        for (auto crystal : crystals) { 
            if (crystal->GetIsActive()) {
                DrawCharacter(crystal->GetPos(), crystal->GetScale(), glm::vec4(0.4f, 0.25f, 0.1f, 1.0f), crystal->GetUpVec(), 0.0f, crystal->GetMeshes());
            }
        }
    }
};

void Renderer::DrawCharacter(const glm::vec3 &pos, float scale, const glm::vec4 &fallbackColor,
    const glm::vec3 &up, float yaw, const std::vector<struct LoadedMesh> *meshes,
    const glm::vec4 *colorOverride) 
{
    GLint locModel = mShader->GetLocModel();
    GLint locObjectColor = mShader->GetLocObjectColor();
    GLint locUseTexture = mShader->GetLocUseTexture();
    GLint locDiffuseTexture = mShader->GetLocDiffuseTexture();

    glm::vec3 upN = glm::normalize(up);
    glm::vec3 worldLeft = glm::normalize(glm::cross(upN, glm::vec3(0, 0, 1)));
    if (glm::length(worldLeft) < 0.01f)
        worldLeft = glm::normalize(glm::cross(upN, glm::vec3(1, 0, 0)));
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

    if (!meshes->empty())
    {
        for (const LoadedMesh &m : *meshes)
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
    std::cout << "not Mesh" << std::endl;
}

void Renderer::DrawGuard(glm::mat4 viewMat, Enemy* enemy) {
    GLint locModel = mShader->GetLocModel();
    GLint locObjectColor = mShader->GetLocObjectColor();
    GLint locUseTexture = mShader->GetLocUseTexture();
    GLint locDiffuseTexture = mShader->GetLocDiffuseTexture();

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

std::vector<glm::mat4> Renderer::GetViews() {
    std::vector<glm::mat4> views;

    bool isOpening = GetGame()->GetGameProgressState()->GetSceneState() == GameProgressState::SceneState::Opening;
    if (isOpening) {
        bool isTalkWithMother = GetGame()->GetUIState()->GetTalkWith() == UIState::TalkWith::Mother;
        bool isTalkWithDoctor = GetGame()->GetUIState()->GetTalkWith() == UIState::TalkWith::Doctor;

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
                
            glm::mat4 view = boat->GetFocusComponent()->GetFocusView();
            views.emplace_back(view);
            players[0]->SetCanMove(false);
        }
    }

    Key* key = currentPlanet->GetKey();
    if (key) {
        if (key->GetFocusComponent()->GetFocusTimer() >= 0.0f) {
            glm::mat4 view = key->GetFocusComponent()->GetFocusView();
            views.emplace_back(view);
            players[0]->SetCanMove(false);
        }
    }
    
    bool isStageClear = mGame->GetGameProgressState()->GetSceneState() == GameProgressState::SceneState::StageClear;
    if (isStageClear) {
        glm::mat4 view = players[0]->getPlayerView(4.0f, true);
        views.emplace_back(view);
    }

    if (views.empty()) {
        glm::mat4 view = players[0]->getPlayerView(10.0f);
        views.emplace_back(view);
        players[0]->SetCanMove(true);
    }    

    bool isPlayer2Joined = mGame->GetIsPlayer2Joined();
    if (isPlayer2Joined) {
        glm::mat4 view2 = players[1]->getPlayerView(4.0f, true);
        views.emplace_back(view2);
    }

    return views;
}

void Renderer::AddImgInfo(std::string path, std::string name) {
    int imgWidth, imgHeight, imgChannels;
    unsigned char* imageData = stbi_load(path.c_str(), &imgWidth, &imgHeight, &imgChannels, STBI_rgb_alpha);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    stbi_image_free(imageData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    mTextures[name] = tex;
}