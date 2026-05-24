#pragma once

#include <string>

class Game;

class GameProgressState {
public:
    enum class SceneState {
        None,
        Title,
        Opening,
        Talking,
        Playing,
        Focusing,
        ShowUI,
        StageClear,
        GameOver,
    };

    GameProgressState(Game* game);

    void SetIsFirstBreak(bool isFirstBreak) { mIsFirstBreak = isFirstBreak; }

    void SetCurrentSceneState(SceneState currentSceneState) { mCurrentSceneState = currentSceneState; }
    void SetNextSceneState(SceneState nextSceneState) { mNextSceneState = nextSceneState; }

    bool GetIsFirstBreak() const { return mIsFirstBreak; }

    SceneState GetSceneState() const { return mCurrentSceneState; }
    SceneState GetNextSceneState() const { return mNextSceneState; }
private:
    bool mIsFirstBreak;

    SceneState mCurrentSceneState;
    SceneState mNextSceneState;

    Game* mGame;
};