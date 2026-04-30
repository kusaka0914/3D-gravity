#include "UIRenderer.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Game.h"
#include "Player.h"
#include "Planet.h"
#include "BoatParts.h"
#include "UIState.h"
#include "NPC.h"
#include "TalkableComponent.h"
#include "GameProgressState.h"
#include "stb_image.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>
#include <memory>

UIRenderer::UIRenderer(Game* game)
    : mGame(game)
    , mShader(game->GetShader())
    , mFont(game->GetFont())
    , mVertexArrays(game->GetVertexArrays())
{
    Initialize();
}

void UIRenderer::Initialize() {
    AddImgInfo("../assets/textures/titleBg.png", "titleBg");
    AddImgInfo("../assets/textures/Opening.png", "Opening");
    AddImgInfo("../assets/textures/textBg.png", "textBg");
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

    glUseProgram(mShader->GetShaderProgram());

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    bool isTitle = GetGame()->GetGameProgressState()->GetSceneState() == GameProgressState::SceneState::Title;
    if (isTitle) {
        DrawTitle();
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        return;
    }

    bool isOpening = GetGame()->GetGameProgressState()->GetSceneState() == GameProgressState::SceneState::Opening;
    if (isOpening) {
        DrawOpening();
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        return;
    }

    bool isStageClear = mGame->GetGameProgressState()->GetSceneState() == GameProgressState::SceneState::StageClear;
    bool isPlaying = mGame->GetGameProgressState()->GetSceneState() == GameProgressState::SceneState::Playing;
    if (!isStageClear && isPlaying){
        DrawDefaultUI();
    } else if (isStageClear) {
        DrawStageClear();
    }

    DrawStateUI();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

void UIRenderer::DrawTitle() {
    DrawTexture(0.0f, 0.0f, mFbWidth, mFbHeight, "titleBg");
    std::string text = "START: A";
    DrawText(mFbWidth / 2, 3 * mFbHeight / 4, 1.0f, text.c_str(), true);
}

void UIRenderer::DrawOpening() {
    switch (GetGame()->GetUIState()->GetTalkWith())
    {
    case UIState::TalkWith::Opening: {
        std::vector<std::string> openingTexts = {
            "ある草原のど真ん中\nスライムはママと仲良く暮らしていました。",
            "ママはいつもお仕事で大忙し\nそれでもスライムを愛しているため一生懸命働いていました。",
            "ある日、スライムはいつものように\nママのお見送りをしていました。"
        };
        int openingUIIndex = GetGame()->GetUIState()->GetTalkUIIndex();
        if (openingUIIndex < openingTexts.size()) {
            DrawTexture(0.0f, 0.0f, mFbWidth, mFbHeight, "Opening");
            DrawTalkUI(openingTexts, openingUIIndex);
            return;
        } else {
            GetGame()->GetUIState()->SetTalkWith("Mother");
            GetGame()->GetUIState()->SetTalkUIIndex(0);
        }
    }
    case UIState::TalkWith::Mother: {
        std::vector<std::string> talkTexts = {
            "スライム: ママ、いってらっしゃい！",
            "ママ: いってきます。\nスライム、いい子にしてるんだよ。",
            "ママ: !?",
            "ママ: ううっ...",
            "ママが突然倒れこんでしまいました。",
            "スライム: どうしたの！？ママ！"
        };
        int talkUIIndex = GetGame()->GetUIState()->GetTalkUIIndex();
        if (talkUIIndex < talkTexts.size()) {
            DrawTalkUI(talkTexts, talkUIIndex);
            return;
        } else {
            GetGame()->GetUIState()->SetTalkUIIndex(0);
            GetGame()->GetUIState()->SetTalkWith("Doctor");
        }
        break;
    }
    case UIState::TalkWith::Doctor: {
        std::vector<std::string> talkTexts = {
            "スライムは急いでドクターを呼びました。",
            "ドクター: これは、私にはどうしても治せない\nとてもとても複雑な病気だよ。",
            "ドクター: きっともう目覚めることはないだろう。",
            "スライム: ほ、ほんとなの、？",
            "スライムは泣き出してしまいました。",
            "スライム: どうにか治す方法はないの？",
            "ドクター: ごめんな、普通の方法じゃ治らないんだ。\nただ...",
            "スライム: ただ...？",
            "ドクター: 一つだけ方法はあるんだが、\nこれはとても難しい方法なんだ。",
            "ドクター: この世界の宇宙になんでも願いを叶えてくれる\n星が存在するという話は聞いたことあるかな？",
            "スライム: 聞いたことないや、、",
            "ドクター: そうか、もしその星を全て集めて、\nお母さんの病気が治ることを願うことができれば...",
            "スライム: じゃあ僕が集めてくるよ！",
            "ドクター: ダメだダメだ！\nおまえさん、宇宙がどれだけ危険なのか分かっているのか！",
            "スライム: じゃあドクターはママを助けてくれるの？",
            "ドクター: ...",
            "スライム: ママを助けられるのは僕しかいない！",
            "そういうとスライムはすぐに家を飛び出し、\nロケットに乗り込んで宇宙へと旅立ちました。"
        };
        int talkUIIndex = GetGame()->GetUIState()->GetTalkUIIndex();
        if (talkUIIndex < talkTexts.size()) {
            DrawTalkUI(talkTexts, talkUIIndex);
            return;
        } else {
            GetGame()->ChangeStage(0);
            GetGame()->GetUIState()->SetTalkWith("None");
            GetGame()->GetUIState()->SetTalkUIIndex(0);
        }
        break;
    }
    }
}

void UIRenderer::DrawTalkUI(const std::vector<std::string>& texts, int index) {
    float bgWidth = 3 * mFbWidth / 4;
    float bgHeight = mFbHeight / 4;
    float bgX = 60.0f;
    float bgY = 50.0f;
    DrawTexture(bgX, bgY, bgWidth, bgHeight, "textBg");

    float textX = bgX + mFbWidth / 1600.0f * 40.0f;
    float textY = bgY + bgHeight / 2 - mFbWidth / 1600.0f * 30.0f;
    float textScale = mFbWidth / 3000.0f;
    glm::vec4 textColor{0.0f, 0.0f, 0.0f, 0.0f};
    DrawText(textX, textY, textScale, texts[index], false, textColor);
}

bool UIRenderer::DrawStateUI() {
    UIState::TutorialKind currentTutorialKind = GetGame()->GetUIState()->GetCurrentTutorialKind();
    switch (currentTutorialKind)
    {
    case UIState::TutorialKind::BoatParts:
        // DrawTutorial();
        return true;
        break;

    case UIState::TutorialKind::Crystal: 
        // DrawCrystalTutorial();
        return true;
    
    case UIState::TutorialKind::Battle:
        DrawBattleTutorial();
        return true;
    
    case UIState::TutorialKind::Break:
        DrawBreakTutorial();
        return true; 

    default:
        break;
    }

    UIState::TalkWith currentTalkWith = GetGame()->GetUIState()->GetTalkWith();
    switch (currentTalkWith)
    {
    case UIState::TalkWith::NPC: {
        NPC* talkingNPC = GetGame()->GetPlayers()[0]->GetTalkingNPC();
        std::vector<std::string> talkTexts = talkingNPC->GetTalkableComponent()->GetTalkTexts();
        int talkUIIndex = GetGame()->GetUIState()->GetTalkUIIndex();
        if (talkUIIndex < talkTexts.size()) {
            DrawTalkUI(talkTexts, talkUIIndex);
            return true;
        } else {
            GetGame()->GetUIState()->SetTalkWith("None");
            GetGame()->GetUIState()->SetTalkUIIndex(0);
            GetGame()->GetGameProgressState()->SetSceneState("Playing");
        }
        return true;
        break;
        }
    default:
        break;
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

    std::vector<NPC*> NPCs = currentPlanet->GetNPCs();
    if (!NPCs.empty()) {
        for (auto NPC : NPCs) {
            if (NPC->GetTalkableComponent()->GetIsTalkable()) {
                DrawTalkableUI();
            }
        }
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

void UIRenderer::DrawTalkableUI() {
    std::string text = "会話: A";
    DrawText(mFbWidth / 2, mFbHeight / 2, 0.5f, text.c_str(), true);
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
    glUniformMatrix4fv(mShader->GetLocModel(), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(mShader->GetLocView(), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(mShader->GetLocProj(), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform1i(mShader->GetLocDiffuseTexture(), 0);
    glUniform1i(mShader->GetLocUseTexture(), 1);
    glUniform3f(mShader->GetLocObjectColor(), color.x, color.y, color.z);

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
        glUniformMatrix4fv(mShader->GetLocModel(), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(mShader->GetLocView(), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(mShader->GetLocProj(), 1, GL_FALSE, glm::value_ptr(proj));
        glUniform1i(mShader->GetLocDiffuseTexture(), 0);
        glUniform1i(mShader->GetLocUseTexture(), 1);
        glUniform3f(mShader->GetLocObjectColor(), color.x, color.y, color.z);

        glBindTexture(GL_TEXTURE_2D, tex);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

void UIRenderer::DrawTexture(float x, float y, float width, float height, std::string textureName) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x + width * 0.5f, y + height * 0.5f, 0.0f))
                    * glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0f));
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
    GLuint tex = mTextures.at(textureName);
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