#include <string>

class GameProgressState {
public:
    enum class SceneState {
        Title,
        Opening,
        Talking,
        Playing,
        ShowUI,
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
        } else if (sceneState == "Talking") {
            mSceneState = SceneState::Talking;
        } else if (sceneState == "Playing") {
            mSceneState = SceneState::Playing;
        } else if (sceneState == "ShowUI") {
            mSceneState = SceneState::ShowUI;
        } else if (sceneState == "StageClear") {
            mSceneState = SceneState::StageClear;
        } else if (sceneState == "GameOver") {
            mSceneState = SceneState::GameOver;
        }
    }

    bool GetIsFirstBreak() const { return mIsFirstBreak; }
    bool GetIsStageClear() const { return mIsStageClear; }
    SceneState GetSceneState() const { return mSceneState; }
private:
    Game* mGame;
    bool mIsFirstBreak;
    bool mIsStageClear;

    SceneState mSceneState;
};