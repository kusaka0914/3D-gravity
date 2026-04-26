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
{}

void Renderer::Draw() {
    GLFWwindow* window = mGame->GetWindow();
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

    std::vector<Player*> players = mGame->GetPlayers();
    bool isPlayer2Joined = mGame->GetIsPlayer2Joined();

    Planet* currentPlanet = players[0]->GetCurrentPlanet();
    std::vector<Boat*> boats = currentPlanet->GetBoats();
    glm::mat4 view;
    if (!boats.empty()) {
        for (auto boat : boats) {
            Key* key = currentPlanet->GetKey();
            if (boat->GetFocusComponent()->GetFocusTimer() >= 0.0f) {
                view = boat->GetFocusComponent()->GetFocusView();
                players[0]->SetCanMove(false);
            } else if (key->GetFocusComponent()->GetFocusTimer() >= 0.0f) {
                view = key->GetFocusComponent()->GetFocusView();
                players[0]->SetCanMove(false);
            } else if (GetGame()->GetGameProgressState()->GetIsStageClear()) {
                view = players[0]->getPlayerView(4.0f, true);
            }
            else {
                view = players[0]->getPlayerView(10.0f);
                players[0]->SetCanMove(true);
            }
        }
    } else {
        if (GetGame()->GetGameProgressState()->GetIsStageClear()) {
            view = players[0]->getPlayerView(4.0f, true);
        }
        else {
            view = players[0]->getPlayerView(10.0f);
            players[0]->SetCanMove(true);
        }
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
        auto planetMeshesByPath = currentStage->GetPlanetMeshesByPath();
        // 惑星描画
        for (size_t i = 0; i < planets.size(); i++)
        {
            glm::mat4 planetModel = glm::translate(glm::mat4(1.0f), planets[i]->GetCenter()) * glm::scale(glm::mat4(1.0f), glm::vec3(planets[i]->GetRadius()));
            glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(planetModel));
            glUniform3f(locObjectColor, planets[i]->GetColor().x, planets[i]->GetColor().y, planets[i]->GetColor().z);
            auto it = planetMeshesByPath.find(planets[i]->GetModelPath());
            if (it != planetMeshesByPath.end() && !it->second.empty())
            {
                for (const LoadedMesh &m : it->second)
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
            else
            {
                // glBindVertexArray(sphereVAO);
                // glUniform1i(locUseTexture, 0);
                // glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
            }
        }

        auto drawCharacter = [&](const glm::vec3 &pos, float scale, const glm::vec3 &fallbackColor,
                                 const glm::vec3 &up, float yaw, const std::vector<LoadedMesh> &meshes,
                                 const glm::vec3 *colorOverride = nullptr)
        {
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
            if (!meshes.empty())
            {
                glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
                for (const LoadedMesh &m : meshes)
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
                        glUniform3f(locObjectColor, colorOverride->x, colorOverride->y, colorOverride->z);
                    }
                    else
                    {
                        glUniform3f(locObjectColor, m.diffuseColor[0], m.diffuseColor[1], m.diffuseColor[2]);
                    }
                    glDrawElements(GL_TRIANGLES, m.indexCount, GL_UNSIGNED_INT, 0);
                }
                glUniform1i(locUseTexture, 0);
            }
            else
            {
                // glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
                // glBindVertexArray(VAO);
                // glm::vec3 c = colorOverride ? *colorOverride : fallbackColor;
                // glUniform3f(locObjectColor, c.x, c.y, c.z);
                // glDrawArrays(GL_TRIANGLES, 0, 3);
            }
        };

        const float playerScale = 0.25f;
        // 1Pの描画
        if (players[0]->GetIsActive()) {
            glm::vec3 up0 = glm::normalize(players[0]->GetPos() - planets[players[0]->GetCurrentPlanetNum()]->GetCenter());
            drawCharacter(players[0]->GetPos(), playerScale, glm::vec3(0.0f, 0.0f, 1.0f), up0, players[0]->GetFacingYaw(), players[0]->GetMeshes());
        }

        // 2Pの描画
        if (isPlayer2Joined)
        {
            glm::vec3 up1 = glm::normalize(players[1]->GetPos() - planets[players[1]->GetCurrentPlanetNum()]->GetCenter());
            drawCharacter(players[1]->GetPos(), playerScale, glm::vec3(1.0f, 0.5f, 0.0f), up1, players[1]->GetFacingYaw(),  players[1]->GetMeshes());
        }

        std::vector<Enemy*> enemies = currentStage->GetPlanets()[players[0]->GetCurrentPlanetNum()]->GetEnemies();
        // 敵描画
        for (size_t ei = 0; ei < enemies.size(); ei++)
        {
            Enemy*& enemy = enemies[ei];
            if (!enemy->GetIsAlive())
                continue;
            std::unordered_map<std::string, std::vector<LoadedMesh>> enemyMeshesByPath = currentStage->GetPlanets()[0]->GetEnemyMeshesByPath();
            auto eit = enemyMeshesByPath.find(enemy->GetModelPath());
            if (eit == enemyMeshesByPath.end() || eit->second.empty())
                eit = enemyMeshesByPath.find("enemy.obj");
            if (eit == enemyMeshesByPath.end() || eit->second.empty())
                continue;
            glm::vec3 enemyUp = glm::normalize(enemy->GetPos() - enemy->GetCurrentPlanet()->GetCenter());
            glm::vec3 toPlayer = glm::normalize(players[0]->GetPos() - enemy->GetPos());
            float enemyFacingYaw = players[0]->getYawFromDirection(enemyUp, toPlayer) + 3.14159265f;
            drawCharacter(enemy->GetPos(), enemy->GetScale(), glm::vec3(0.0f, 1.0f, 0.0f), enemyUp, enemyFacingYaw, eit->second);
            // 敵の頭上にID（1始まり）をビルボード表示
            if (mFont)
            {
                std::unordered_map<std::string, std::pair<GLuint, glm::ivec2>> textTextureCache;
                // 文字列→テクスチャ（敵ID表示用、キャッシュ付き）
                auto getTextTexture = [&](const std::string &s) -> std::pair<GLuint, glm::ivec2>
                {
                    if (!mFont || s.empty())
                        return {0, {0, 0}};
                    auto it = textTextureCache.find(s);
                    if (it != textTextureCache.end())
                        return it->second;
                    SDL_Color white = {255, 255, 255, 255};
                    SDL_Surface *surf = TTF_RenderText_Blended(mFont, s.c_str(), white);
                    if (!surf)
                        return {0, {0, 0}};
                    SDL_Surface *rgba = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
                    SDL_FreeSurface(surf);
                    if (!rgba)
                        return {0, {0, 0}};
                    int tw = rgba->w, th = rgba->h;
                    GLuint tex;
                    glGenTextures(1, &tex);
                    glBindTexture(GL_TEXTURE_2D, tex);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba->pixels);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    SDL_FreeSurface(rgba);
                    textTextureCache[s] = {tex, {tw, th}};
                    return {tex, {tw, th}};
                };
                auto [texId, texSize] = getTextTexture(std::to_string(enemies[ei]->GetBreakCount()));
                if (texId != 0 && texSize.x > 0 && texSize.y > 0)
                {
                    glm::vec3 camPos(glm::inverse(viewMat)[3]);
                    glm::vec3 quadCenter = enemy->GetPos() + enemyUp * 0.8f;
                    glm::vec3 forward = glm::normalize(camPos - quadCenter);
                    glm::vec3 right = glm::normalize(glm::cross(enemyUp, forward));
                    if (glm::length(right) < 0.01f)
                        right = glm::normalize(glm::cross(enemyUp, glm::vec3(0, 0, 1)));
                    glm::vec3 upQuad = glm::cross(forward, right);
                    // 敵のどれくらい上にラベルを描画するのか
                    const float enemyLabelHeight = 0.5f;
                    float w = enemyLabelHeight * static_cast<float>(texSize.x) / static_cast<float>(texSize.y);
                    glm::mat4 billboard(1.0f);
                    billboard[0] = glm::vec4(right * w, 0.0f);
                    billboard[1] = glm::vec4(upQuad * enemyLabelHeight, 0.0f);
                    billboard[2] = glm::vec4(forward, 0.0f);
                    billboard[3] = glm::vec4(quadCenter, 1.0f);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glDepthMask(GL_FALSE);
                    glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(billboard));
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, texId);
                    glUniform1i(locUseTexture, 1);
                    glUniform3f(locObjectColor, 1.0f, 1.0f, 1.0f);
                    mVertexArrays.at("text")->SetActive();
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                    glUniform1i(locUseTexture, 0);
                    glDepthMask(GL_TRUE);
                    glDisable(GL_BLEND);
                }
            }
        }

        // 鍵描画
        Planet* currentPlanet = players[0]->GetCurrentPlanet();
        Key* key = currentPlanet->GetKey();
        if (key) {
            if (key->GetIsActive())
            {
                const float keyScale = 2.0f;
                const glm::vec3 keyColor(0.85f, 0.65f, 0.13f); // 金色
                glm::vec3 keyUp = glm::normalize(key->GetPos() - currentPlanet->GetCenter());
                drawCharacter(key->GetPos(), keyScale, keyColor, keyUp, 0.0f, key->GetMeshes(), &keyColor);
            }
        }

        std::vector<Boat*> boats = currentPlanet->GetBoats();
        // ボート描画
        if (!boats.empty()) {
            for (auto boat : boats) {
                if (boat->GetIsActive())
                {
                    const float boatScale = 0.8f;
                    glm::vec3 boatUp = glm::normalize(boat->GetPos() - currentPlanet->GetCenter());
                    drawCharacter(boat->GetPos(), boatScale, glm::vec3(0.4f, 0.25f, 0.1f), boatUp, 0.0f, boat->GetMeshes());
                }
            }
        }

        // ボートのかけら描画
        std::vector<BoatParts*> boatParts = currentPlanet->GetBoatParts();
        if (!boatParts.empty()) {
            for (auto parts : boatParts) { 
                if (parts->GetIsActive()) {
                    const float boatPartsScale = 1.0f;
                    glm::vec3 boatPartsUp = glm::normalize(parts->GetPos() - currentPlanet->GetCenter());
                    drawCharacter(parts->GetPos(), boatPartsScale, glm::vec3(0.4f, 0.25f, 0.1f), boatPartsUp, 0.0f, parts->GetMeshes());
                }
            }
        }

        // スター描画
        Star* star = currentPlanet->GetStar();
        if (star) {
        if (star->GetIsActive())
            {
                glm::vec3 starUp = glm::normalize(star->GetPos() - currentPlanet->GetCenter());
                glm::vec3 starColor(1.0f, 0.9f, 0.2f);
                const float starScale = 0.3f;
                drawCharacter(star->GetPos(), starScale, starColor, starUp, 0.0f, star->GetMeshes());
            }
        }

        // クリスタル描画
        std::vector<Crystal*> crystals = currentPlanet->GetCrystals();
        if (!crystals.empty()) {
            for (auto crystal : crystals) { 
                if (crystal->GetIsActive()) {
                    glm::vec3 crystalUp = glm::normalize(crystal->GetPos() - currentPlanet->GetCenter());
                    drawCharacter(crystal->GetPos(), crystal->GetScale(), glm::vec3(0.4f, 0.25f, 0.1f), crystalUp, 0.0f, crystal->GetMeshes());
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