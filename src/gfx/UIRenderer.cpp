#include "UIRenderer.h"
#include "gfx/UIShader.h"
#include "VertexArray.h"
#include "Game.h"
#include "actor/Player.h"
#include "actor/Planet.h"
#include "actor/BoatParts.h"
#include "state/UIState.h"
#include "actor/NPC.h"
#include "component/TalkableComponent.h"
#include "state/GameProgressState.h"
#include "system/UILoader.h"
#include "stb_image.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>
#include <memory>

UIRenderer::UIRenderer(Game* game)
    : mGame(game)
    , mUIShader(game->GetUIShader())
    , mFont(game->GetFont())
    , mVertexArrays(game->GetVertexArrays())
    , mUILoader(game->GetUILoader())
{
    Initialize();
}

void UIRenderer::Initialize() {
    AddImgInfo("../assets/textures/titleBg.png", "titleBg");
    AddImgInfo("../assets/textures/opening.png", "opening");
    AddImgInfo("../assets/textures/textBg.png", "textBg");
    AddImgInfo("../assets/textures/slime.png", "slime");
    AddImgInfo("../assets/textures/hp.png", "hp");
    AddImgInfo("../assets/textures/special.png", "special");
}

void UIRenderer::AddImgInfo(std::string path, std::string name) {
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

void UIRenderer::Draw() {
    GLFWwindow* window = mGame->GetWindow();
    glfwGetFramebufferSize(window, &mFbWidth, &mFbHeight);

    glUseProgram(mUIShader->GetShaderProgram());

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    bool isTitle = GetGame()->GetGameProgressState()->GetSceneState() == GameProgressState::SceneState::Title;
    if (isTitle)
        DrawTitle();

    bool isOpening = GetGame()->GetGameProgressState()->GetSceneState() == GameProgressState::SceneState::Opening;
    if (isOpening)
        DrawOpening();

    bool isPlaying = mGame->GetGameProgressState()->GetSceneState() == GameProgressState::SceneState::Playing;
    if (isPlaying)
        DrawDefaultUI();

    DrawStateUI();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

void UIRenderer::DrawTitle() {
    auto titleBgTextureInfo = mUILoader->GetTextureInfo("title", "bgTexture");
    if (titleBgTextureInfo)
        DrawTexture(titleBgTextureInfo->x, titleBgTextureInfo->y, mFbWidth * titleBgTextureInfo->widthRatio, mFbHeight * titleBgTextureInfo->heightRatio, "titleBg");

    auto startTextInfo = mUILoader->GetTextInfo("title", "startText");
    if (startTextInfo)
        DrawText(mFbWidth * startTextInfo->xRatio, mFbHeight * startTextInfo->yRatio, startTextInfo->scaleRatio, startTextInfo->texts[0], true);
}

void UIRenderer::DrawOpening() {
    auto talkWith = GetGame()->GetUIState()->GetTalkWith();

    switch (talkWith)
    {
        case UIState::TalkWith::Opening: {
            DrawOpeningIntro();
            break;
        }
        case UIState::TalkWith::Mother: {
            DrawOpeningTalkWithMother();
            break;
        }
        case UIState::TalkWith::Doctor: {
            DrawOpeningTalkWithDoctor();
            break;
        }
    }
}

void UIRenderer::DrawOpeningIntro() {
    auto openingBgTextureInfo = mUILoader->GetTextureInfo("opening", "bgTexture");
    auto openingTalkTextInfo = mUILoader->GetTextInfo("opening", "openingText");

    if (!openingBgTextureInfo || !openingTalkTextInfo) return;

    int talkUIIndex = GetGame()->GetUIState()->GetTalkUIIndex();
    std::vector<std::string> talkTexts = openingTalkTextInfo->texts;

    if (talkUIIndex < talkTexts.size()) {
        DrawTexture(openingBgTextureInfo->x, openingBgTextureInfo->y, mFbWidth * openingBgTextureInfo->widthRatio, mFbHeight * openingBgTextureInfo->heightRatio, "opening");
        DrawTalkUI(talkTexts, talkUIIndex);
        return;
    }
    
    GetGame()->GetUIState()->SetTalkWith("Mother");
    GetGame()->GetUIState()->SetTalkUIIndex(0);
}

void UIRenderer::DrawOpeningTalkWithMother() {
    auto talkWithMotherTextInfo = mUILoader->GetTextInfo("opening", "talkWithMotherText");
    if (!talkWithMotherTextInfo) return;

    int talkUIIndex = GetGame()->GetUIState()->GetTalkUIIndex();
    std::vector<std::string> talkTexts = talkWithMotherTextInfo->texts;

    if (talkUIIndex < talkTexts.size()) {
        DrawTalkUI(talkTexts, talkUIIndex);
        return;
    }

    GetGame()->GetUIState()->SetTalkUIIndex(0);
    GetGame()->GetUIState()->SetTalkWith("Doctor");
}

void UIRenderer::DrawOpeningTalkWithDoctor() {
    auto talkWithDoctorTextInfo = mUILoader->GetTextInfo("opening", "talkWithDoctorText");
    if (!talkWithDoctorTextInfo) return;

    int talkUIIndex = GetGame()->GetUIState()->GetTalkUIIndex();
    std::vector<std::string> talkTexts = talkWithDoctorTextInfo->texts;

    if (talkUIIndex < talkTexts.size() && talkUIIndex >= 0) {
        DrawTalkUI(talkTexts, talkUIIndex);
    }
    else if (talkUIIndex >= static_cast<int>(talkTexts.size())) {
        GetGame()->GetUIState()->SetTalkUIIndex(0);
        GetGame()->SetFadeInTimer(1.0f);
        GetGame()->GetGameProgressState()->SetNextSceneState("Playing");
    }
}

void UIRenderer::DrawDefaultUI() {
    DrawOperationSupportUI();

    std::vector<Player*> players = mGame->GetPlayers();
    Planet* currentPlanet = players[0]->GetCurrentPlanet();
    std::vector<NPC*> NPCs = currentPlanet->GetNPCs();

    if (!NPCs.empty()) {
        for (auto NPC : NPCs) {
            if (!NPC->GetTalkableComponent()) continue;
            if (NPC->GetTalkableComponent()->GetIsTalkable())
                DrawTalkableUI();  
        }  
    }  
    
    int currentStageNum = GetGame()->GetCurrentStageNum();
    if (currentStageNum == 0) return;

    DrawHpUI();

    float specialAttackCooldownRemaining = players[0]->GetSpecialAttackCooldownRemaining();

    if (specialAttackCooldownRemaining <= 0.0f)
        DrawSpecialAttackUI();
    
    std::vector<BoatParts*> boatParts = currentPlanet->GetBoatParts();

    if (!boatParts.empty())
        DrawRemainPartsUI(); 
}

void UIRenderer::DrawOperationSupportUI() {
    auto operationSupportTextInfo = mUILoader->GetTextInfo("default", "operationSupportText");
    if (!operationSupportTextInfo) return;
    
    DrawText(operationSupportTextInfo->x, mFbHeight - operationSupportTextInfo->y, mFbWidth * operationSupportTextInfo->scaleRatio, operationSupportTextInfo->texts[0], false);
}

void UIRenderer::DrawHpUI() {
    auto hpTextureInfo = mUILoader->GetTextureInfo("default", "hpTexture");
    if (!hpTextureInfo) return;

    std::vector<Player*> players = mGame->GetPlayers();
    int hp = players[0]->GetHp() / 10;

    float hpX = mFbWidth * hpTextureInfo->xRatio;
    while (hp > 0) {
        DrawTexture(hpX, mFbWidth * hpTextureInfo->yRatio, mFbWidth * hpTextureInfo->widthRatio, mFbWidth * hpTextureInfo->heightRatio, "hp");
        hpX += mFbWidth / 28;
        hp--;
    }
}

void UIRenderer::DrawSpecialAttackUI() {
    auto specialAttackTextureInfo = mUILoader->GetTextureInfo("default", "specialAttackTexture");
    if (!specialAttackTextureInfo) return;

    DrawTexture(mFbWidth * specialAttackTextureInfo->xRatio, mFbWidth * specialAttackTextureInfo->yRatio, mFbWidth * specialAttackTextureInfo->widthRatio, mFbWidth * specialAttackTextureInfo->heightRatio, "special");

    auto specialAttackTextInfo = mUILoader->GetTextInfo("default", "specialAttackText");
    if (!specialAttackTextInfo) return;

    DrawText(mFbWidth * specialAttackTextInfo->xRatio, mFbHeight * specialAttackTextInfo->yRatio, mFbWidth * specialAttackTextInfo->scaleRatio, specialAttackTextInfo->texts[0], false);
}

void UIRenderer::DrawTalkableUI() {
    auto talkableTextInfo = mUILoader->GetTextInfo("default", "talkableText");
    if (!talkableTextInfo) return;

    DrawText(mFbWidth * talkableTextInfo->xRatio, mFbHeight * talkableTextInfo->yRatio, mFbWidth * talkableTextInfo->scaleRatio, talkableTextInfo->texts[0], true);
}

void UIRenderer::DrawRemainPartsUI() {
    auto remainPartsTextInfo = mUILoader->GetTextInfo("default", "remainPartsText");
    if (!remainPartsTextInfo) return;

    std::vector<Player*> players = mGame->GetPlayers();
    Planet* currentPlanet = players[0]->GetCurrentPlanet();
    std::vector<BoatParts*> boatParts = currentPlanet->GetBoatParts();
    
    int remainBoatParts = 0;
    for(auto parts : boatParts) {
        if (!parts->GetIsActive()) continue;
        remainBoatParts++;
    }

    if (remainBoatParts == 0) return;

    std::string remainText = remainPartsTextInfo->texts[0] + std::to_string(remainBoatParts);
    DrawText(mFbWidth - mFbWidth * remainPartsTextInfo->xRatio, mFbWidth * remainPartsTextInfo->yRatio, mFbWidth * remainPartsTextInfo->scaleRatio, remainText, false);
}

void UIRenderer::DrawStateUI() {
    UIState::TutorialKind currentTutorialKind = GetGame()->GetUIState()->GetCurrentTutorialKind();

    switch (currentTutorialKind)
    {
        case UIState::TutorialKind::Battle:
            DrawBattleTutorial();
            break;
        
        case UIState::TutorialKind::Break:
            DrawBreakTutorial();
            break; 

        default:
            break;
    }

    UIState::TalkWith currentTalkWith = GetGame()->GetUIState()->GetTalkWith();

    switch (currentTalkWith)
    {
        case UIState::TalkWith::NPC: 
            DrawTalkWithNPC();
            break;

        default:
            break;
    }

    bool isStageClear = mGame->GetGameProgressState()->GetSceneState() == GameProgressState::SceneState::StageClear;
    if (isStageClear) 
        DrawStageClear();

    float fadeInTimer = GetGame()->GetFadeInTimer();
    float alpha;

    if (fadeInTimer >= 0.0f) 
        alpha = 1.0f - fadeInTimer;
    else 
        alpha = 1.0f + fadeInTimer;

    if (alpha > 0.0f) 
        DrawBG(mFbWidth, mFbHeight, 0.0f, 0.0f, {0.0f, 0.0f, 0.0f, alpha});

    // ローディング描画はフェードイン背景より後に描画する必要があるため以下動かさない
    bool isLoading = GetGame()->GetIsChangeStage() && GetGame()->GetFadeInTimer() <= 0.1f;
    if (isLoading) 
        DrawLoading();
}

void UIRenderer::DrawBattleTutorial() {
    auto battleTutorialTextInfo = mUILoader->GetTextInfo("state", "battleTutorialText");
    if (!battleTutorialTextInfo) return;

    std::vector<std::string> battleTutorialTexts = battleTutorialTextInfo->texts;
    int tutorialUIIndex = GetGame()->GetUIState()->GetTalkUIIndex();

    if (tutorialUIIndex < battleTutorialTexts.size()) {
        DrawTalkUI(battleTutorialTexts, tutorialUIIndex);
        return;
    }

    GetGame()->GetUIState()->SetTalkUIIndex(0);
    GetGame()->GetUIState()->SetCurrentTutorialKind("None");
    GetGame()->GetGameProgressState()->SetSceneState("Playing");
}

void UIRenderer::DrawBreakTutorial() {
    auto breakTutorialTextInfo = mUILoader->GetTextInfo("state", "breakTutorialText");
    if (!breakTutorialTextInfo) return;

    std::vector<std::string> breakTutorialTexts = breakTutorialTextInfo->texts;
    int tutorialUIIndex = GetGame()->GetUIState()->GetTalkUIIndex();

    if (tutorialUIIndex < breakTutorialTexts.size()) {
        DrawTalkUI(breakTutorialTexts, tutorialUIIndex);
        return;
    }

    GetGame()->GetUIState()->SetTalkUIIndex(0);
    GetGame()->GetUIState()->SetCurrentTutorialKind("None");
    GetGame()->GetGameProgressState()->SetSceneState("Playing");
}

void UIRenderer::DrawStageClear() {
    auto stageClearTextInfo = mUILoader->GetTextInfo("state", "stageClearText");
    if (!stageClearTextInfo) return;

    DrawText(mFbWidth * stageClearTextInfo->xRatio, stageClearTextInfo->y, stageClearTextInfo->scaleRatio, stageClearTextInfo->texts[0], true);
}

void UIRenderer::DrawTalkUI(const std::vector<std::string>& texts, int index) {
    auto talkBgTextureInfo = mUILoader->GetTextureInfo("state", "talkBgTexture");
    if (!talkBgTextureInfo) return;

    DrawTexture(talkBgTextureInfo->x, talkBgTextureInfo->y, mFbWidth * talkBgTextureInfo->widthRatio, mFbHeight * talkBgTextureInfo->heightRatio, "textBg");
    
    auto talkTextInfo = mUILoader->GetTextInfo("state", "talkText");
    if (!talkTextInfo) return;

    glm::vec4 textColor{0.0f, 0.0f, 0.0f, 0.0f};
    DrawText(mFbWidth * talkTextInfo->xRatio, mFbHeight * talkTextInfo->yRatio, mFbWidth * talkTextInfo->scaleRatio, texts[index], false, textColor);
}

void UIRenderer::DrawTalkWithNPC() {
    NPC* talkingNPC = GetGame()->GetPlayers()[0]->GetTalkingNPC();
    std::vector<std::string> talkTexts = talkingNPC->GetTalkableComponent()->GetTalkTexts();
    int talkUIIndex = GetGame()->GetUIState()->GetTalkUIIndex();

    if (talkUIIndex < talkTexts.size()) {
        DrawTalkUI(talkTexts, talkUIIndex);
        return;
    } 

    GetGame()->GetUIState()->SetTalkWith("None");
    GetGame()->GetUIState()->SetTalkUIIndex(0);
    GetGame()->GetGameProgressState()->SetSceneState("Playing");
}

void UIRenderer::DrawLoading() {
    auto loadingTextInfo = mUILoader->GetTextInfo("state", "loadingText");
    if (!loadingTextInfo) return;

    DrawText(loadingTextInfo->x, mFbHeight - loadingTextInfo->y, loadingTextInfo->scaleRatio, loadingTextInfo->texts[0], false);

    auto loadingTextureInfo = mUILoader->GetTextureInfo("state", "loadingTexture");
    if (!loadingTextureInfo) return;

    DrawTexture(loadingTextureInfo->x, mFbHeight - loadingTextureInfo->y, loadingTextureInfo->width, loadingTextureInfo->height, "slime");
}

void UIRenderer::DrawBG(float width, float height, float x, float y, std::vector<GLfloat> color)
{
    // 背景の描画
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x + width * 0.5f, y + height * 0.5f, 0.0f))
                * glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0f));
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 proj = glm::ortho(0.0f, (float)mFbWidth, (float)mFbHeight, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(mUIShader->GetLocModel(), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(mUIShader->GetLocView(), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(mUIShader->GetLocProj(), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform1i(mUIShader->GetLocUseTexture(), 0);
    glUniform4fv(mUIShader->GetLocObjectColor(), 1, color.data());
    mVertexArrays.at("text")->SetActive();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void UIRenderer::DrawText(float x, float y, float scale, std::string message, bool isCenterBase, glm::vec4 color)
{
    std::string message1 = message;
    std::string message2 = "";
    int newline = message.find("\n");
    bool isNewLine = newline != std::string::npos;
    if (isNewLine) {
        message1 = message.substr(0, newline);
        message2 = message.substr(newline + 1);
    } else {
        newline = message.find("\\n");
        isNewLine = newline != std::string::npos;
        if (isNewLine) {
            message1 = message.substr(0, newline);
            message2 = message.substr(newline + 2);
        }
    }
    
    SDL_Color textColor{static_cast<Uint8>(color.x), static_cast<Uint8>(color.y), static_cast<Uint8>(color.z), static_cast<Uint8>(color.w)};
    SDL_Surface* surf = TTF_RenderUTF8_Blended(mFont, message1.c_str(), textColor);

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
    } else if (isNewLine && isCenterBase) {
        model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y - 20, 0.0f))
                    * glm::scale(glm::mat4(1.0f), glm::vec3(textWidth, textHeight, 1.0f));
    } else if (isNewLine) {
        model = glm::translate(glm::mat4(1.0f), glm::vec3(x + textWidth * 0.5f, y + textHeight * 0.5f - 20.0f, 0.0f))
                    * glm::scale(glm::mat4(1.0f), glm::vec3(textWidth, textHeight, 1.0f));
    } else {
        model = glm::translate(glm::mat4(1.0f), glm::vec3(x + textWidth * 0.5f, y + textHeight * 0.5f, 0.0f))
                    * glm::scale(glm::mat4(1.0f), glm::vec3(textWidth, textHeight, 1.0f));
    }
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 proj = glm::ortho(0.0f, (float)mFbWidth, (float)mFbHeight, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(mUIShader->GetLocModel(), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(mUIShader->GetLocView(), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(mUIShader->GetLocProj(), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform1i(mUIShader->GetLocDiffuseTexture(), 0);
    glUniform1i(mUIShader->GetLocUseTexture(), 1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    mVertexArrays.at("text")->SetActive();
    glDrawArrays(GL_TRIANGLES, 0, 6);

    if (isNewLine) {
        surf = TTF_RenderUTF8_Blended(mFont, message2.c_str(), textColor);
            if (!surf) return;
        rgba = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(surf);
        if (!rgba) return;

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba->w, rgba->h, 0,
                    GL_RGBA, GL_UNSIGNED_BYTE, rgba->pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        textWidth = (float)rgba->w * scale, textHeight = (float)rgba->h * scale;
        SDL_FreeSurface(rgba);

        if (isCenterBase) {
            model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y + 40.0f, 0.0f))
                        * glm::scale(glm::mat4(1.0f), glm::vec3(textWidth, textHeight, 1.0f));
        }else{
            model = glm::translate(glm::mat4(1.0f), glm::vec3(x + textWidth * 0.5f, y + textHeight * 0.5f + 40.0f, 0.0f))
                        * glm::scale(glm::mat4(1.0f), glm::vec3(textWidth, textHeight, 1.0f));
        }
        view = glm::mat4(1.0f);
        proj = glm::ortho(0.0f, (float)mFbWidth, (float)mFbHeight, 0.0f, -1.0f, 1.0f);
        glUniformMatrix4fv(mUIShader->GetLocModel(), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(mUIShader->GetLocView(), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(mUIShader->GetLocProj(), 1, GL_FALSE, glm::value_ptr(proj));
        glUniform1i(mUIShader->GetLocDiffuseTexture(), 0);
        glUniform1i(mUIShader->GetLocUseTexture(), 1);

        glBindTexture(GL_TEXTURE_2D, tex);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

void UIRenderer::DrawTexture(float x, float y, float width, float height, std::string textureName) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x + width * 0.5f, y + height * 0.5f, 0.0f))
                    * glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0f));
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 proj = glm::ortho(0.0f, (float)mFbWidth, (float)mFbHeight, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(mUIShader->GetLocModel(), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(mUIShader->GetLocView(), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(mUIShader->GetLocProj(), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform1i(mUIShader->GetLocDiffuseTexture(), 0);
    glUniform1i(mUIShader->GetLocUseTexture(), 1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);
    GLuint tex = mTextures.at(textureName);
    glBindTexture(GL_TEXTURE_2D, tex);

    mVertexArrays.at("text")->SetActive();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}