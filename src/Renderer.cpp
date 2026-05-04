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

    std::vector<Player*> players = mGame->GetPlayers();
    bool isPlayer2Joined = mGame->GetIsPlayer2Joined();

    Planet* currentPlanet = players[0]->GetCurrentPlanet();
    std::vector<Boat*> boats = currentPlanet->GetBoats();
    Key* key = currentPlanet->GetKey();
    glm::mat4 view;
    bool isStageClear = mGame->GetGameProgressState()->GetSceneState() == GameProgressState::SceneState::StageClear;
    if (!boats.empty()) {
        for (auto boat : boats) {
            if (boat->GetFocusComponent()->GetFocusTimer() >= 0.0f) {
                view = boat->GetFocusComponent()->GetFocusView();
                players[0]->SetCanMove(false);
            } else if (isStageClear) {
                view = players[0]->getPlayerView(4.0f, true);
            }
            else {
                view = players[0]->getPlayerView(10.0f);
                players[0]->SetCanMove(true);
            }
        }
    }
    if (key) {
        if (key->GetFocusComponent()->GetFocusTimer() >= 0.0f) {
            view = key->GetFocusComponent()->GetFocusView();
            players[0]->SetCanMove(false);
        } else if (isStageClear) {
            view = players[0]->getPlayerView(4.0f, true);
        }
        else {
            view = players[0]->getPlayerView(10.0f);
            players[0]->SetCanMove(true);
        }
    }
    if (boats.empty() && !key) {
        if (isStageClear) {
            view = players[0]->getPlayerView(4.0f, true);
        }
        else {
            view = players[0]->getPlayerView(10.0f);
            players[0]->SetCanMove(true);
        }
    }
    bool isTalkWithMother = GetGame()->GetUIState()->GetTalkWith() == UIState::TalkWith::Mother;
    bool isTalkWithDoctor = GetGame()->GetUIState()->GetTalkWith() == UIState::TalkWith::Doctor;
    if (isTalkWithMother) {
        view = glm::lookAt(glm::vec3(-2.0f, 4.0f, -2.0f), glm::vec3(4.0f, 2.0f, 4.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    } else if (isTalkWithDoctor) {
        view = glm::lookAt(glm::vec3(3.0f, 4.0f, 1.0f), glm::vec3(-4.0f, 2.0f, -4.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    glm::mat4 view2P = isPlayer2Joined ? players[1]->getPlayerView(12.0f) : view;

    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // これから描画するときにどのプログラムを使うのか設定
    glUseProgram(mShader->GetShaderProgram());

    auto drawScene = [&](const glm::mat4 &viewMat, const glm::mat4 &projMat)
    {
        GLint locModel = mShader->GetLocModel();
        GLint locView = mShader->GetLocView();
        GLint locProj = mShader->GetLocProj();
        GLint locObjectColor = mShader->GetLocObjectColor();
        GLint locUseTexture = mShader->GetLocUseTexture();
        GLint locDiffuseTexture = mShader->GetLocDiffuseTexture();

        // CPU側のMVPをシェーダーのそれぞれのuniformに渡して使えるようにしている
        // 惑星描画
        glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(viewMat));
        glUniformMatrix4fv(locProj, 1, GL_FALSE, glm::value_ptr(projMat));

        Stage* currentStage = mGame->GetCurrentStage();
        std::vector<Planet*> planets = currentStage->GetPlanets();
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

        const float playerScale = 0.25f;
        // 1Pの描画
        if (players[0]->GetIsActive()) {
            DrawCharacter(players[0]->GetPos(), playerScale, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), players[0]->GetUpVec(), players[0]->GetFacingYaw(), players[0]->GetMeshes());
        }

        // 2Pの描画
        if (isPlayer2Joined)
        {
            DrawCharacter(players[1]->GetPos(), playerScale, glm::vec4(1.0f, 0.5f, 0.0f, 1.0f), players[1]->GetUpVec(), players[1]->GetFacingYaw(),  players[1]->GetMeshes());
        }

        Planet* currentPlanet = players[0]->GetCurrentPlanet();
        if (currentPlanet){
            std::vector<Enemy*> enemies = currentPlanet->GetEnemies();
            // 敵描画
            for (auto enemy : enemies)
            {
                if (!enemy->GetIsAlive())
                    continue;
                
                glm::vec3 enemyUp = enemy->GetUpVec();
                glm::vec3 toPlayer = glm::normalize(players[0]->GetPos() - enemy->GetPos());
                float enemyFacingYaw = players[0]->getYawFromDirection(enemyUp, toPlayer) + 3.14159265f;

                DrawCharacter(enemy->GetPos(), enemy->GetScale(), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), enemyUp, enemyFacingYaw, enemy->GetMeshes());

                // 敵の頭上にID（1始まり）をビルボード表示
                GLuint texId = mTextures["guard"];
                if (enemy->GetBreakCount() == 0)
                    continue;
                if (texId != 0)
                {
                    glm::vec3 camPos(glm::inverse(viewMat)[3]);
                    glm::vec3 quadCenter = enemy->GetPos() + enemyUp * 0.8f;
                    glm::vec3 forward = glm::normalize(camPos - quadCenter);
                    glm::vec3 right = glm::normalize(glm::cross(enemyUp, forward));
                    if (glm::length(right) < 0.01f)
                        right = glm::normalize(glm::cross(enemyUp, glm::vec3(0, 0, 1)));
                    glm::vec3 upQuad = glm::cross(forward, right);
                    glm::vec3 drawPos = enemy->GetPos() + enemyUp * 0.8f;
                    
                    const float enemyLabelHeight = 0.5f;

                    glm::mat4 billboard(1.0f);
                    billboard[0] = glm::vec4(right * enemyLabelHeight, 0.0f);
                    billboard[1] = glm::vec4(-upQuad * enemyLabelHeight, 0.0f);
                    billboard[2] = glm::vec4(forward, 0.0f);
                    billboard[3] = glm::vec4(drawPos, 1.0f);

                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glDepthMask(GL_FALSE);

                    glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(billboard));
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, texId);
                    glUniform1i(locUseTexture, 1);
                    mVertexArrays.at("text")->SetActive();
                    glDrawArrays(GL_TRIANGLES, 0, 6);

                    glUniform1i(locUseTexture, 0);
                    glDepthMask(GL_TRUE);
                    glDisable(GL_BLEND);
                }
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

    if (!isPlayer2Joined)
    {
        glViewport(0, 0, fbWidth, fbHeight);
        float aspect = static_cast<float>(fbWidth) / static_cast<float>(fbHeight);
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
        drawScene(view, proj);
    }
    else
    {
        float halfW = fbWidth * 0.5f;
        float aspectHalf = halfW / static_cast<float>(fbHeight);
        glm::mat4 projHalf = glm::perspective(glm::radians(45.0f), aspectHalf, 0.1f, 100.0f);
        glViewport(0, 0, static_cast<GLsizei>(halfW), fbHeight);
        drawScene(view, projHalf);
        glViewport(static_cast<GLsizei>(halfW), 0, static_cast<GLsizei>(halfW), fbHeight);
        drawScene(view2P, projHalf);
    }
}

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
    }
    else
    {
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        // glBindVertexArray(VAO);
        glm::vec4 c = colorOverride ? *colorOverride : fallbackColor;
        glUniform4f(locObjectColor, c.x, c.y, c.z, c.w);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        std::cout << "not Mesh" << std::endl;
    }
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