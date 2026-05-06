#include <string>

class GameProgressState {
public:
    enum class SceneState {
        None,
        Title,
        Opening,
        Talking,
        Playing,
        ShowUI,
        StageClear,
        GameOver,
    };
    GameProgressState(class Game* game);
    void SetIsFirstBreak(bool isFirstBreak) { mIsFirstBreak = isFirstBreak; }
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
        } else if (sceneState == "None") {
            mSceneState = SceneState::None;
        }
    }

    void SetNextSceneState(std::string sceneState) {
        if (sceneState == "Title") {
            mNextSceneState = SceneState::Title;
        } else if (sceneState == "Opening") {
            mNextSceneState = SceneState::Opening;
        } else if (sceneState == "Talking") {
            mNextSceneState = SceneState::Talking;
        } else if (sceneState == "Playing") {
            mNextSceneState = SceneState::Playing;
        } else if (sceneState == "ShowUI") {
            mNextSceneState = SceneState::ShowUI;
        } else if (sceneState == "StageClear") {
            mNextSceneState = SceneState::StageClear;
        } else if (sceneState == "GameOver") {
            mNextSceneState = SceneState::GameOver;
        } else if (sceneState == "None") {
            mNextSceneState = SceneState::None;
        }
    }

    bool GetIsFirstBreak() const { return mIsFirstBreak; }
    SceneState GetSceneState() const { return mSceneState; }
    SceneState GetNextSceneState() const { return mNextSceneState; }
private:
    Game* mGame;
    bool mIsFirstBreak;

    SceneState mSceneState;
    SceneState mNextSceneState;
};