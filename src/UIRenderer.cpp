#include "UIRenderer.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Game.h"
#include "Player.h"
#include "Planet.h"
#include "BoatParts.h"
#include "UIState.h"
#include "GameProgressState.h"
#include "stb_image.h"
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

void UIRenderer::Draw() {
    GLFWwindow* window = mGame->GetWindow();
    glfwGetFramebufferSize(window, &mFbWidth, &mFbHeight);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    bool isStageClear = mGame->GetGameProgressState()->GetIsStageClear();
    if (!isStageClear){
        DrawDefaultUI();
    }

    bool isStateUIActive = DrawStateUI();
    if (isStateUIActive) {
        GetGame()->GetUIState()->SetIsUIActive(true);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

bool UIRenderer::DrawStateUI() {
    bool isTutorialActive = mGame->GetUIState()->GetIsTutorialActive();
    if (isTutorialActive) {
        DrawTutorial();
        return true;
    }

    bool isCrystalTutorialActive = mGame->GetUIState()->GetIsCrystalTutorialActive();
    if (isCrystalTutorialActive) {
        DrawCrystalTutorial();
        return true;
    }

    bool isBattleTutorialActive = mGame->GetUIState()->GetIsBattleTutorialActive();
    if (isBattleTutorialActive) {
        DrawBattleTutorial();
        return true;
    }

    bool isBreakTutorialActive = mGame->GetUIState()->GetIsBreakTutorialActive();
    if (isBreakTutorialActive) {
        DrawBreakTutorial();
        return true;
    }

    bool isStageClear = mGame->GetGameProgressState()->GetIsStageClear();
    if (isStageClear) {
        DrawStageClear();
        return true;
    }

    return false;
}

void UIRenderer::DrawDefaultUI() {
    DrawOperationSupportUI();

    DrawHpUI();


    std::vector<Player*> players = mGame->GetPlayers();
    if (players[0]->GetSpecialAttackCooldownRemaining() <= 0.0f) {
        DrawSpecialAttackUI();
    }
    
    Planet* currentPlanet = players[0]->GetCurrentPlanet();
    std::vector<BoatParts*> boatParts = currentPlanet->GetBoatParts();

    if (!boatParts.empty()){
        DrawRemainPartsUI();
    }
}

void UIRenderer::DrawOperationSupportUI() {
    DrawText(20, mFbHeight - 60, 0.5f, "A:ジャンプ B:回避 X:攻撃 Y:広範囲攻撃 L:スペシャル攻撃 空中でX長押し→離す:溜め攻撃", false);
}

void UIRenderer::DrawHpUI() {
    std::vector<Player*> players = mGame->GetPlayers();
    int Hp = players[0]->GetHp();
    std::string HpText = "体力: " + std::to_string(Hp);
    DrawText(mFbWidth - 200, 40, 0.5f, HpText.c_str(), false);
}

void UIRenderer::DrawSpecialAttackUI() {
    std::string text = "スペシャル攻撃OK!";
    DrawText(mFbWidth / 2, 40, 0.5f, text.c_str(), true);
}

void UIRenderer::DrawBG(float width, float height, float x, float y, std::vector<GLfloat> color)
{
    // 背景の描画
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x + width * 0.5f, y + height * 0.5f, 0.0f))
                * glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0f));
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 proj = glm::ortho(0.0f, (float)mFbWidth, (float)mFbHeight, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(mShader->GetLocModel(), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(mShader->GetLocView(), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(mShader->GetLocProj(), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform1i(mShader->GetLocUseTexture(), 0);
    glUniform3fv(mShader->GetLocObjectColor(), 1, color.data());
    mVertexArrays.at("text")->SetActive();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void UIRenderer::DrawText(float x, float y, float scale, const char* message, bool isCenterBase)
{
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

    float textWidth = (float)rgba->w * scale, textHeight = (float)rgba->h * scale;
    SDL_FreeSurface(rgba);

    glm::mat4 model;
    if (isCenterBase) {
        model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f))
                    * glm::scale(glm::mat4(1.0f), glm::vec3(textWidth, textHeight, 1.0f));
    }else{
        model = glm::translate(glm::mat4(1.0f), glm::vec3(x + textWidth * 0.5f, y + textHeight * 0.5f, 0.0f))
                    * glm::scale(glm::mat4(1.0f), glm::vec3(textWidth, textHeight, 1.0f));
    }
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 proj = glm::ortho(0.0f, (float)mFbWidth, (float)mFbHeight, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(mShader->GetLocModel(), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(mShader->GetLocView(), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(mShader->GetLocProj(), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform1i(mShader->GetLocDiffuseTexture(), 0);
    glUniform1i(mShader->GetLocUseTexture(), 1);
    glUniform3f(mShader->GetLocObjectColor(), 1.0f, 1.0f, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    mVertexArrays.at("text")->SetActive();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void UIRenderer::DrawTexture(float x, float y, float scale, const char* path) {
    int imgWidth, imgHeight, imgChannels;
    unsigned char* imageData = stbi_load(path, &imgWidth, &imgHeight, &imgChannels, STBI_rgb_alpha);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    stbi_image_free(imageData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x + imgWidth * scale * 0.5f, y + imgHeight * scale * 0.5f, 0.0f))
                    * glm::scale(glm::mat4(1.0f), glm::vec3(imgWidth * scale, imgHeight * scale, 1.0f));
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 proj = glm::ortho(0.0f, (float)mFbWidth, (float)mFbHeight, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(mShader->GetLocModel(), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(mShader->GetLocView(), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(mShader->GetLocProj(), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform1i(mShader->GetLocDiffuseTexture(), 0);
    glUniform1i(mShader->GetLocUseTexture(), 1);
    glUniform3f(mShader->GetLocObjectColor(), 1.0f, 1.0f, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    mVertexArrays.at("text")->SetActive();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void UIRenderer::DrawTutorial() {
    float bgWidth = mFbWidth * 0.7f;
    float bgHeight = mFbHeight * 0.7f;
    float fbWidthHalf = mFbWidth * 0.5f;
    float fbHeightHalf = mFbHeight * 0.5f;
    float bgX = fbWidthHalf - bgWidth / 2;
    float bgY = fbHeightHalf - bgHeight / 2;
    float textX = bgX + bgWidth /2;
    DrawBG(bgWidth, bgHeight, bgX, bgY, {0.0f, 0.0f, 0.0f});
    DrawText(textX, bgY + 100, 0.5f, "ロケットのかけらを5個集めよう！", true);
    // DrawTexture(100, 100, 0.5, "../assets/textures/grassTex.png");
    DrawText(textX, bgY + bgHeight - 380, 0.5f, "この惑星にはロケットのかけらが5個散らばっています。", true);
    DrawText(textX, bgY + bgHeight - 320, 0.5f, "ジャンプや回避、攻撃を駆使して全てのかけらを集めてください。", true);
    DrawText(textX, bgY + bgHeight - 260, 0.5f, "全てのかけらを集めることで次の惑星に行くためのロケットが完成します。", true);
}

void UIRenderer::DrawCrystalTutorial() {
    float bgWidth = mFbWidth * 0.7f;
    float bgHeight = mFbHeight * 0.7f;
    float fbWidthHalf = mFbWidth * 0.5f;
    float fbHeightHalf = mFbHeight * 0.5f;
    float bgX = fbWidthHalf - bgWidth / 2;
    float bgY = fbHeightHalf - bgHeight / 2;
    float textX = bgX + bgWidth /2;
    DrawBG(bgWidth, bgHeight, bgX, bgY, {0.0f, 0.0f, 0.0f});
    DrawText(textX, bgY + 100, 0.5f, "クリスタルを攻撃して破壊しよう！", true);
    // DrawTexture(100, 100, 0.5, "../assets/textures/grassTex.png");
    DrawText(textX, bgY + bgHeight - 380, 0.5f, "クリスタルは攻撃することで破壊できます。", true);
    DrawText(textX, bgY + bgHeight - 320, 0.5f, "クリスタルの中にはアイテムが隠れていることがあるので", true);
    DrawText(textX, bgY + bgHeight - 260, 0.5f, "見つけたら積極的に破壊してみてください。", true);
    DrawText(textX, bgY + bgHeight - 200, 0.5f, "大きいクリスタルは空中溜め攻撃で破壊できますよ。", true);
    // DrawText(textX, bgY + bgHeight - 140, 0.5f, "空中でAボタンを長押しし、移動が止まった状態（チャージ完了状態）で離すことで出せる", true);
    // DrawText(textX, bgY + bgHeight - 80, 0.5f, "溜め攻撃を当てれば一発で壊れますよ！", true);
}

void UIRenderer::DrawBattleTutorial() {
    float bgWidth = mFbWidth * 0.7f;
    float bgHeight = mFbHeight * 0.7f;
    float fbWidthHalf = mFbWidth * 0.5f;
    float fbHeightHalf = mFbHeight * 0.5f;
    float bgX = fbWidthHalf - bgWidth / 2;
    float bgY = fbHeightHalf - bgHeight / 2;
    float textX = bgX + bgWidth /2;
    DrawBG(bgWidth, bgHeight, bgX, bgY, {0.0f, 0.0f, 0.0f});
    DrawText(textX, bgY + 100, 0.5f, "攻撃してガードを壊そう！", true);
    // DrawTexture(100, 100, 0.5, "../assets/textures/grassTex.png");
    DrawText(textX, bgY + bgHeight - 380, 0.5f, "攻撃: X 広範囲攻撃: Y", true);
    DrawText(textX, bgY + bgHeight - 320, 0.5f, "敵はガードを所持していて、連続して3回攻撃を当てると1つ壊れます。", true);
    DrawText(textX, bgY + bgHeight - 260, 0.5f, "全てのガードを破壊するとブレイク状態になります。", true);
    DrawText(textX, bgY + bgHeight - 200, 0.5f, "まずは一度ブレイクしてみましょう。", true);
}

void UIRenderer::DrawBreakTutorial() {
    float bgWidth = mFbWidth * 0.7f;
    float bgHeight = mFbHeight * 0.7f;
    float fbWidthHalf = mFbWidth * 0.5f;
    float fbHeightHalf = mFbHeight * 0.5f;
    float bgX = fbWidthHalf - bgWidth / 2;
    float bgY = fbHeightHalf - bgHeight / 2;
    float textX = bgX + bgWidth /2;
    DrawBG(bgWidth, bgHeight, bgX, bgY, {0.0f, 0.0f, 0.0f});
    DrawText(textX, bgY + 100, 0.5f, "溜め攻撃を繰り出そう！", true);
    // DrawTexture(100, 100, 0.5, "../assets/textures/grassTex.png");
    DrawText(textX, bgY + bgHeight - 380, 0.5f, "ナイスブレイク！", true);
    DrawText(textX, bgY + bgHeight - 320, 0.5f, "ブレイクすると敵が空中に打ち上げられます。", true);
    DrawText(textX, bgY + bgHeight - 260, 0.5f, "空中の敵を攻撃するにはジャンプしてXボタンを長押しします。", true);
    DrawText(textX, bgY + bgHeight - 200, 0.5f, "最大まで溜めた状態で離すことで高火力な溜め攻撃が繰り出せます。", true);
}

void UIRenderer::DrawStageClear() {
    float textX = mFbWidth * 0.5f;
    DrawText(textX, 300, 1.0f, "ステージクリア！", true);
}

void UIRenderer::DrawRemainPartsUI() {
    std::vector<Player*> players = mGame->GetPlayers();
    Planet* currentPlanet = players[0]->GetCurrentPlanet();
    std::vector<BoatParts*> boatParts = currentPlanet->GetBoatParts();
    
    int remainBoatParts = 0;
    for(auto parts : boatParts) {
        if (parts->GetIsActive()) {
            remainBoatParts++;
        }
    }
    std::string remainText = "残りのかけら数: " + std::to_string(remainBoatParts);
    if (remainBoatParts != 0){
        DrawText(40, 40, 0.5f, remainText.c_str(), false);
    }
}