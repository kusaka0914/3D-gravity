#include "UIRenderer.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Game.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>
#include <memory>

UIRenderer::UIRenderer(Game* game)
    : mGame(game)
    , mShader(game->GetShader())
    , mFont(game->GetFont())
    , mVertexArrays(game->GetVertexArrays())
{}

void UIRenderer::DrawTextBox(int fbWidth, int fbHeight, float padding, float panelWidth, float panelHeight, float panelX, float panelY, std::vector<GLfloat> panelColor, const char* message)
{
    DrawPanel(fbWidth, fbHeight, panelWidth, panelHeight, panelX, panelY, panelColor);    
    DrawText(fbWidth, fbHeight, panelX + padding, panelY + padding, message);
}

void UIRenderer::DrawPanel(int fbWidth, int fbHeight, float panelWidth, float panelHeight, float panelX, float panelY, std::vector<GLfloat> panelColor)
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    // 背景の描画
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(panelX + panelWidth * 0.5f, panelY + panelHeight * 0.5f, 0.0f))
                * glm::scale(glm::mat4(1.0f), glm::vec3(panelWidth, panelHeight, 1.0f));
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 proj = glm::ortho(0.0f, (float)fbWidth, (float)fbHeight, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(mShader->GetLocModel(), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(mShader->GetLocView(), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(mShader->GetLocProj(), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform1i(mShader->GetLocUseTexture(), 0);
    glUniform3fv(mShader->GetLocObjectColor(), 1, panelColor.data());
    mVertexArrays.at("text")->SetActive();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void UIRenderer::DrawText(int fbWidth, int fbHeight, float textX, float textY, const char* message)
{
    // --- 白文字（SDL_ttf → テクスチャ）---
    SDL_Color white{255, 255, 255, 255};
    SDL_Surface* surf = TTF_RenderUTF8_Blended(mFont, message, white);
    if (!surf) return;
    SDL_Surface* rgba = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surf);
    if (!rgba) return;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba->w, rgba->h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, rgba->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    float textWidth = (float)rgba->w, textHeight = (float)rgba->h;
    SDL_FreeSurface(rgba);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(textX + textWidth * 0.5f, textY + textHeight * 0.5f, 0.0f))
                    * glm::scale(glm::mat4(1.0f), glm::vec3(textWidth, textHeight, 1.0f));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUniformMatrix4fv(mShader->GetLocModel(), 1, GL_FALSE, glm::value_ptr(model));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(mShader->GetLocDiffuseTexture(), 0);
    glUniform1i(mShader->GetLocUseTexture(), 1);
    glUniform3f(mShader->GetLocObjectColor(), 1.0f, 1.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glUniform1i(mShader->GetLocUseTexture(), 0);
    glDisable(GL_BLEND);
    glDeleteTextures(1, &tex);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}