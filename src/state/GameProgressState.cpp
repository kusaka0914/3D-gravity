#include "GameProgressState.h"
#include "Game.h"

GameProgressState::GameProgressState(Game* game)
    :mGame(game)
    ,mIsFirstBreak(false)
    ,mCurrentSceneState(SceneState::Playing)
    ,mNextSceneState(SceneState::None)
{

}