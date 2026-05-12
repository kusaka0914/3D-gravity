#include "GameProgressState.h"
#include "Game.h"

GameProgressState::GameProgressState(Game* game)
    :mGame(game)
    ,mIsFirstBreak(false)
    ,mSceneState(SceneState::Playing)
    ,mNextSceneState(SceneState::None)
{

}