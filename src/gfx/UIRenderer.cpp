#include "UIRenderer.h"
#include "gfx/UIShader.h"
#include "VertexArray.h"
#include "Game.h"
#include "actor/Player.h"
#include "actor/Planet.h"
#include "state/UIState.h"
#include "system/SceneSystem.h"
#include "actor/NPC.h"
#include <glm/gtc/type_ptr.hpp>

UIRenderer::UIRenderer(Game* game)
    : Renderer(game)
{
    Initialize();
}

UIRenderer::~UIRenderer() = default;

void UIRenderer::Initialize() {
    mUIShaderUnique = std::make_unique<UIShader>();
    mUIShader = mUIShaderUnique.get();

    mUILoadSystemUnique = std::make_unique<UILoadSystem>();
    mUILoadSystem = mUILoadSystemUnique.get();

    if (!mUIShader->GetShaderProgram()) {
        glfwTerminate();
        return;
    }

    RegisterUITextures();
}

void UIRenderer::RegisterUITextures() {
    RegisterTexture("../assets/textures/titleBg.png", "titleBg");
    RegisterTexture("../assets/textures/opening.png", "opening");
    RegisterTexture("../assets/textures/textBg.png", "textBg");
    RegisterTexture("../assets/textures/slime.png", "slime");
    RegisterTexture("../assets/textures/hp.png", "hp");
    RegisterTexture("../assets/textures/special.png", "special");
    RegisterTexture("../assets/textures/skyBox.png", "skyBox");
    RegisterTexture("../assets/textures/jewel.png", "jewel");
}

void UIRenderer::Draw() {
    glfwGetFramebufferSize(mGame->GetWindow(), &mFbWidth, &mFbHeight);

    glUseProgram(mUIShader->GetShaderProgram());

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    bool isTitle = mGame->GetSceneSystem()->IsTitle();
    if (isTitle)
        DrawTitle();

    bool isOpening = mGame->GetSceneSystem()->IsOpening();
    if (isOpening)
        DrawOpening();
    
    bool isGameOver = mGame->GetSceneSystem()->IsGameOver();
    if (isGameOver)
        DrawGameOver();

    bool isPlaying = mGame->GetSceneSystem()->IsPlaying();
    UIState::TutorialKind currentTutorialKind = mGame->GetSceneSystem()->GetCurrentTutorialKind();
    if (isPlaying || currentTutorialKind == UIState::TutorialKind::Jewel)
        DrawDefaultUI();

    DrawStateUI();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

void UIRenderer::DrawTitle() {
    auto titleBgTextureInfo = mUILoadSystem->GetTextureInfo("title", "bgTexture");
    if (titleBgTextureInfo)
        DrawTexture(titleBgTextureInfo->x, titleBgTextureInfo->y, mFbWidth * titleBgTextureInfo->widthRatio, mFbHeight * titleBgTextureInfo->heightRatio, "titleBg");

    auto startTextInfo = mUILoadSystem->GetTextInfo("title", "startText");
    if (startTextInfo)
        DrawText(mFbWidth * startTextInfo->xRatio, mFbHeight * startTextInfo->yRatio, mFbWidth * startTextInfo->scaleRatio, startTextInfo->texts[0], true);
}

void UIRenderer::DrawOpening() {
    auto currentTalkWith = mGame->GetSceneSystem()->GetCurrentTalkWith();

    switch (currentTalkWith)
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
        default:
            break;
    }
}

void UIRenderer::DrawGameOver() {
    DrawBG(mFbWidth, mFbHeight, 0.0f, 0.0f, {0.0f, 0.0f, 0.0f, 0.5f});

    auto gameOverTextInfo = mUILoadSystem->GetTextInfo("gameOver", "gameOverText");
    if (!gameOverTextInfo) return;

    DrawText(mFbWidth * gameOverTextInfo->xRatio, mFbHeight * gameOverTextInfo->yRatio, mFbWidth * gameOverTextInfo->scaleRatio, gameOverTextInfo->texts[0], true);

    auto restartTextInfo = mUILoadSystem->GetTextInfo("gameOver", "restartText");
    if (!restartTextInfo) return;

    DrawText(mFbWidth * restartTextInfo->xRatio, mFbHeight * restartTextInfo->yRatio, mFbWidth * restartTextInfo->scaleRatio, restartTextInfo->texts[0], true);
}

void UIRenderer::DrawOpeningIntro() {
    auto openingBgTextureInfo = mUILoadSystem->GetTextureInfo("opening", "bgTexture");
    const UILoadSystem::TextInfo* openingTalkTextInfo = mUILoadSystem->GetTextInfo("opening", "openingText");

    if (!openingBgTextureInfo || !openingTalkTextInfo) return;

    const int talkUIIndex = mGame->GetSceneSystem()->GetTalkUIIndex();
    const std::vector<std::string>& talkTexts = openingTalkTextInfo->texts;

    if (talkUIIndex < talkTexts.size()) {
        DrawTexture(openingBgTextureInfo->x, openingBgTextureInfo->y, mFbWidth * openingBgTextureInfo->widthRatio, mFbHeight * openingBgTextureInfo->heightRatio, "opening");
        DrawTalkUI(openingTalkTextInfo, talkUIIndex);
        return;
    }
    
    mGame->GetSceneSystem()->GetUIState()->StartTalkWith(UIState::TalkWith::Mother);
}

void UIRenderer::DrawOpeningTalkWithMother() {
    const UILoadSystem::TextInfo* talkWithMotherTextInfo = mUILoadSystem->GetTextInfo("opening", "talkWithMotherText");
    if (!talkWithMotherTextInfo) return;

    const int talkUIIndex = mGame->GetSceneSystem()->GetTalkUIIndex();
    const std::vector<std::string>& talkTexts = talkWithMotherTextInfo->texts;

    if (talkUIIndex < talkTexts.size()) {
        DrawTalkUI(talkWithMotherTextInfo, talkUIIndex);
        return;
    }

    mGame->GetSceneSystem()->GetUIState()->StartTalkWith(UIState::TalkWith::Doctor);
}

void UIRenderer::DrawOpeningTalkWithDoctor() {
    const UILoadSystem::TextInfo* talkWithDoctorTextInfo = mUILoadSystem->GetTextInfo("opening", "talkWithDoctorText");
    if (!talkWithDoctorTextInfo) return;

    const int talkUIIndex = mGame->GetSceneSystem()->GetTalkUIIndex();
    const std::vector<std::string>& talkTexts = talkWithDoctorTextInfo->texts;

    if (talkUIIndex >= 0 && talkUIIndex < static_cast<int>(talkTexts.size())) {
        DrawTalkUI(talkWithDoctorTextInfo, talkUIIndex);
        return;
    }

    if (talkUIIndex >= static_cast<int>(talkTexts.size())) {
        mGame->GetSceneSystem()->StartFadeIn();
    }
}

void UIRenderer::DrawDefaultUI() {
    DrawOperationSupportUI();

    const std::vector<Player*>& players = mGame->GetPlayers();
    NPC* talkableNPC = players[0]->GetTalkableNPC();
    if (talkableNPC) {
        if (talkableNPC->GetIsTalkable())
            DrawTalkableUI();  
    }
    
    if (mGame->GetCurrentStageNum() == 0) return;

    DrawHpUI();
    DrawJewelUI();
    
    Planet* currentPlanet = players[0]->GetCurrentPlanet();
    if (!currentPlanet->GetBoatParts().empty())
        DrawRemainPartsUI(); 
}

void UIRenderer::DrawOperationSupportUI() {
    bool isOperationUIShow = mGame->GetSceneSystem()->GetUIState()->GetIsOperationUIShow();
    if (isOperationUIShow) {
        auto operationSupportTextInfo = mUILoadSystem->GetTextInfo("default", "operationSupportText");
        if (!operationSupportTextInfo) return;
        
        DrawText(mFbWidth * operationSupportTextInfo->xRatio, mFbHeight - mFbHeight * operationSupportTextInfo->yRatio, mFbWidth * operationSupportTextInfo->scaleRatio, operationSupportTextInfo->texts[0], false);
        return;
    }

    auto operationSupportHiddenTextInfo = mUILoadSystem->GetTextInfo("default", "operationSupportHiddenText");
    if (!operationSupportHiddenTextInfo) return;
    
    DrawText(mFbWidth * operationSupportHiddenTextInfo->xRatio, mFbHeight - mFbHeight * operationSupportHiddenTextInfo->yRatio, mFbWidth * operationSupportHiddenTextInfo->scaleRatio, operationSupportHiddenTextInfo->texts[0], false);
}

void UIRenderer::DrawHpUI() {
    auto hpTextureInfo = mUILoadSystem->GetTextureInfo("default", "hpTexture");
    if (!hpTextureInfo) return;

    int hp = mGame->GetPlayers()[0]->GetHp();
    if (hp == 3) {
        DrawBG(mFbWidth, mFbHeight, 0.0f, 0.0f, {1.0f, 0.0f, 0.0f, 0.05f});
    }
    if (hp == 2) {
        DrawBG(mFbWidth, mFbHeight, 0.0f, 0.0f, {1.0f, 0.0f, 0.0f, 0.1f});
    }
    if (hp == 1) {
        DrawBG(mFbWidth, mFbHeight, 0.0f, 0.0f, {1.0f, 0.0f, 0.0f, 0.2f});
    }

    float hpX = mFbWidth * hpTextureInfo->xRatio;
    float hpY = mFbWidth * hpTextureInfo->yRatio;
    while (hp > 0) {
        hp--;
        DrawTexture(hpX, hpY, mFbWidth * hpTextureInfo->widthRatio, mFbWidth * hpTextureInfo->heightRatio, "hp");
        hpX += mFbWidth / 28;
    }
}

void UIRenderer::DrawJewelUI() {
    float jewel = mGame->GetPlayers()[0]->GetJewel();
    if (jewel <= 0) return;

    auto jewelTextureInfo = mUILoadSystem->GetTextureInfo("default", "jewelTexture");
    if (!jewelTextureInfo) return;

    float jewelX = mFbWidth * jewelTextureInfo->xRatio;
    while(jewel > 0) {
        DrawTexture(jewelX, mFbWidth * jewelTextureInfo->yRatio, mFbWidth * jewelTextureInfo->widthRatio, mFbWidth * jewelTextureInfo->heightRatio, "jewel");
        jewelX += mFbWidth / 20;
        jewel--;
    }
}

void UIRenderer::DrawTalkableUI() {
    auto talkableTextInfo = mUILoadSystem->GetTextInfo("default", "talkableText");
    if (!talkableTextInfo) return;

    DrawText(mFbWidth * talkableTextInfo->xRatio, mFbHeight * talkableTextInfo->yRatio, mFbWidth * talkableTextInfo->scaleRatio, talkableTextInfo->texts[0], true);
}

void UIRenderer::DrawRemainPartsUI() {
    auto remainPartsTextInfo = mUILoadSystem->GetTextInfo("default", "remainPartsText");
    if (!remainPartsTextInfo) return;

    Planet* currentPlanet = mGame->GetPlayers()[0]->GetCurrentPlanet();
    const int remainBoatPartsCount = currentPlanet->GetRemainBoatPartsCount();
    if (remainBoatPartsCount == 0) return;

    std::string remainText = remainPartsTextInfo->texts[0] + std::to_string(remainBoatPartsCount);
    DrawText(mFbWidth - mFbWidth * remainPartsTextInfo->xRatio, mFbWidth * remainPartsTextInfo->yRatio, mFbWidth * remainPartsTextInfo->scaleRatio, remainText, false);
}

void UIRenderer::DrawStateUI() {
    UIState::TutorialKind currentTutorialKind = mGame->GetSceneSystem()->GetCurrentTutorialKind();

    switch (currentTutorialKind)
    {
        case UIState::TutorialKind::Battle:
            DrawBattleTutorial();
            break;
        
        case UIState::TutorialKind::Break:
            DrawBreakTutorial();
            break;

        case UIState::TutorialKind::Jewel:
            DrawJewelTutorial();
            break;

        default:
            break;
    }

    UIState::TalkWith currentTalkWith = mGame->GetSceneSystem()->GetCurrentTalkWith();

    switch (currentTalkWith)
    {
        case UIState::TalkWith::NPC: 
            DrawTalkWithNPC();
            break;

        default:
            break;
    }

    bool isStageClear = mGame->GetSceneSystem()->IsStageClear();
    if (isStageClear) 
        DrawStageClear();

    float fadeInTimer = mGame->GetSceneSystem()->GetFadeTimer();
    float alpha;

    if (fadeInTimer >= 0.0f) 
        alpha = 1.0f - fadeInTimer;
    else 
        alpha = 1.0f + fadeInTimer;

    if (alpha > 0.0f) 
        DrawBG(mFbWidth, mFbHeight, 0.0f, 0.0f, {0.0f, 0.0f, 0.0f, alpha});

    // ローディング描画はフェードイン背景より後に描画する必要があるため以下動かさない
    bool isLoading = mGame->GetSceneSystem()->GetHasPendingStageChange() && mGame->GetSceneSystem()->GetFadeTimer() <= 0.1f;
    if (isLoading) 
        DrawLoading();
}

void UIRenderer::DrawBattleTutorial() {
    const UILoadSystem::TextInfo* battleTutorialTextInfo = mUILoadSystem->GetTextInfo("state", "battleTutorialText");
    if (!battleTutorialTextInfo) return;

    std::vector<std::string> battleTutorialTexts = battleTutorialTextInfo->texts;
    int tutorialUIIndex = mGame->GetSceneSystem()->GetTalkUIIndex();

    if (tutorialUIIndex < battleTutorialTexts.size()) {
        DrawTalkUI(battleTutorialTextInfo, tutorialUIIndex);
        return;
    }

    mGame->StartPlayingScene();
    mGame->GetSceneSystem()->GetUIState()->FinishTutorial();
}

void UIRenderer::DrawBreakTutorial() {
    const UILoadSystem::TextInfo* breakTutorialTextInfo = mUILoadSystem->GetTextInfo("state", "breakTutorialText");
    if (!breakTutorialTextInfo) return;

    std::vector<std::string> breakTutorialTexts = breakTutorialTextInfo->texts;
    int tutorialUIIndex = mGame->GetSceneSystem()->GetTalkUIIndex();

    if (tutorialUIIndex < breakTutorialTexts.size()) {
        DrawTalkUI(breakTutorialTextInfo, tutorialUIIndex);
        return;
    }

    mGame->StartPlayingScene();
    mGame->GetSceneSystem()->GetUIState()->FinishTutorial();
}

void UIRenderer::DrawJewelTutorial() {
    const UILoadSystem::TextInfo* jewelTutorialTextInfo = mUILoadSystem->GetTextInfo("state", "jewelTutorialText");
    if (!jewelTutorialTextInfo) return;

    std::vector<std::string> jewelTutorialTexts = jewelTutorialTextInfo->texts;
    int tutorialUIIndex = mGame->GetSceneSystem()->GetTalkUIIndex();

    if (tutorialUIIndex < jewelTutorialTexts.size()) {
        DrawTalkUI(jewelTutorialTextInfo, tutorialUIIndex);
        return;
    }

    mGame->StartPlayingScene();
    mGame->GetSceneSystem()->GetUIState()->FinishTutorial();
}

void UIRenderer::DrawStageClear() {
    auto stageClearTextInfo = mUILoadSystem->GetTextInfo("state", "stageClearText");
    if (!stageClearTextInfo) return;

    DrawText(mFbWidth * stageClearTextInfo->xRatio, mFbHeight * stageClearTextInfo->yRatio, mFbWidth * stageClearTextInfo->scaleRatio, stageClearTextInfo->texts[0], true);
}

void UIRenderer::DrawTalkUI(const std::vector<std::string>& texts, int index) {
    auto talkBgTextureInfo = mUILoadSystem->GetTextureInfo("state", "talkBgTexture");
    if (!talkBgTextureInfo) return;

    DrawTexture(mFbWidth * talkBgTextureInfo->xRatio, mFbHeight * talkBgTextureInfo->yRatio, mFbWidth * talkBgTextureInfo->widthRatio, mFbHeight * talkBgTextureInfo->heightRatio, "textBg");
    
    auto talkTextInfo = mUILoadSystem->GetTextInfo("state", "talkText");
    if (!talkTextInfo) return;

    glm::vec4 textColor{0.0f, 0.0f, 0.0f, 255.0f};
    DrawText(mFbWidth * talkTextInfo->xRatio, mFbHeight * talkTextInfo->yRatio, mFbWidth * talkTextInfo->scaleRatio, texts[index], false, textColor);
}

void UIRenderer::DrawTalkUI(const UILoadSystem::TextInfo* textInfo, int index) {
    auto talkBgTextureInfo = mUILoadSystem->GetTextureInfo("state", "talkBgTexture");
    if (!talkBgTextureInfo) return;

    float textureMarginX = 0.0275f;
    float textureMarginY = 0.0845f;
    DrawTexture(mFbWidth * (textInfo->xRatio - textureMarginX), mFbHeight * (textInfo->yRatio - textureMarginY), mFbWidth * talkBgTextureInfo->widthRatio, mFbHeight * talkBgTextureInfo->heightRatio, "textBg");

    glm::vec4 textColor{0.0f, 0.0f, 0.0f, 255.0f};
    DrawText(mFbWidth * textInfo->xRatio, mFbHeight * textInfo->yRatio, mFbWidth * textInfo->scaleRatio, textInfo->texts[index], false, textColor);
}

void UIRenderer::DrawTalkWithNPC() {
    NPC* talkableNPC = mGame->GetPlayers()[0]->GetTalkableNPC();
    std::vector<std::string> talkTexts = talkableNPC->GetTalkTexts();
    int talkUIIndex = mGame->GetSceneSystem()->GetTalkUIIndex();

    if (talkUIIndex < talkTexts.size()) {
        DrawTalkUI(talkTexts, talkUIIndex);
        return;
    } 

    mGame->StartPlayingScene();
    mGame->GetSceneSystem()->GetUIState()->FinishTalkWith();
}

void UIRenderer::DrawLoading() {
    auto loadingTextInfo = mUILoadSystem->GetTextInfo("state", "loadingText");
    if (!loadingTextInfo) return;

    DrawText(mFbWidth * loadingTextInfo->xRatio, mFbHeight - mFbHeight * loadingTextInfo->yRatio, mFbWidth * loadingTextInfo->scaleRatio, loadingTextInfo->texts[0], false);

    auto loadingTextureInfo = mUILoadSystem->GetTextureInfo("state", "loadingTexture");
    if (!loadingTextureInfo) return;

    DrawTexture(mFbWidth * loadingTextureInfo->xRatio, mFbHeight - mFbHeight * loadingTextureInfo->yRatio, mFbWidth * loadingTextureInfo->widthRatio, mFbHeight * loadingTextureInfo->heightRatio, "slime");
}

void UIRenderer::DrawSkyBox() {
    glfwGetFramebufferSize(mGame->GetWindow(), &mFbWidth, &mFbHeight);

    glUseProgram(mUIShader->GetShaderProgram());

    DrawTexture(0.0f, 0.0f, static_cast<float>(mFbWidth), static_cast<float>(mFbHeight), "skyBox");
}

void UIRenderer::DrawBG(float width, float height, float x, float y, std::vector<GLfloat> color)
{
    glUseProgram(mUIShader->GetShaderProgram());
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x + width * 0.5f, y + height * 0.5f, 0.0f))
                * glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0f));
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 proj = glm::ortho(0.0f, (float)mFbWidth, (float)mFbHeight, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(mUIShader->GetLocModel(), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(mUIShader->GetLocView(), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(mUIShader->GetLocProj(), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform1i(mUIShader->GetLocUseTexture(), 0);
    glUniform4fv(mUIShader->GetLocObjectColor(), 1, color.data());

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mVertexArrays.at("text")->SetActive();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void UIRenderer::DrawText(float x, float y, float scale, std::string message, bool isCenterBase, glm::vec4 color) {
    glUseProgram(mUIShader->GetShaderProgram());
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
    } else if (isNewLine) {
        model = glm::translate(glm::mat4(1.0f), glm::vec3(x + textWidth * 0.5f, y + textHeight * 0.5f - mFbHeight * 0.0222f, 0.0f))
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

    glDeleteTextures(1, &tex);
    tex = 0;

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
            model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y + mFbHeight * 0.0444f, 0.0f))
                        * glm::scale(glm::mat4(1.0f), glm::vec3(textWidth, textHeight, 1.0f));
        }else{
            model = glm::translate(glm::mat4(1.0f), glm::vec3(x + textWidth * 0.5f, y + textHeight * 0.5f + mFbHeight * 0.0444f, 0.0f))
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

        glDeleteTextures(1, &tex);
        tex = 0;
    }
}

void UIRenderer::DrawTexture(float x, float y, float width, float height, std::string textureName) {
    glUseProgram(mUIShader->GetShaderProgram());
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