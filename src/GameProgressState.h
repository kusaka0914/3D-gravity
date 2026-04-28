#include <string>

class GameProgressState {
public:
    enum class SceneState {
        Title,
        Opening,
        Playing,
        StageClear,
        GameOver
    };
    GameProgressState(class Game* game);
    void SetIsFirstBreak(bool isFirstBreak) { mIsFirstBreak = isFirstBreak; }
    void SetIsStageClear(bool isStageClear) { mIsStageClear = isStageClear; }
    void SetSceneState(std::string sceneState) {
        if (sceneState == "Title") {
            mSceneState = SceneState::Title;
        } else if (sceneState == "Opening") {
            mSceneState = SceneState::Opening;
        } else if (sceneState == "Playing") {
            mSceneState = SceneState::Playing;
        } else if (sceneState == "StageClear") {
            mSceneState = SceneState::StageClear;
        } else if (sceneState == "GameOver") {
            mSceneState = SceneState::GameOver;
        }
    }
    // void SetIsTutorialActive(bool isTutorialActive) { mIsTutorialActive = isTutorialActive; }
    // void SetIsCrystalTutorialActive(bool isCrystalTutorialActive) { mIsCrystalTutorialActive = isCrystalTutorialActive; }
    // void SetIsBattleTutorialActive(bool isBattleTutorialActive) { mIsBattleTutorialActive = isBattleTutorialActive; }
    // void SetIsBattleTutorialShown(bool isBattleTutorialShown) { mIsBattleTutorialShown = isBattleTutorialShown; }

    bool GetIsFirstBreak() const { return mIsFirstBreak; }
    bool GetIsStageClear() const { return mIsStageClear; }
    SceneState GetSceneState() const { return mSceneState; }
    // bool GetIsTutorialActive() const { return mIsTutorialActive; }
    // bool GetIsCrystalTutorialActive() const { return mIsCrystalTutorialActive; }
    // bool GetIsBattleTutorialActive() const { return mIsBattleTutorialActive; }
    // bool GetIsBattleTutorialShown() const { return mIsBattleTutorialShown; }
private:
    Game* mGame;
    bool mIsFirstBreak;
    bool mIsStageClear;

    SceneState mSceneState;
    // bool mIsTutorialActive;
    // bool mIsCrystalTutorialActive;
    // bool mIsBattleTutorialActive;
    // bool mIsBattleTutorialShown;
};