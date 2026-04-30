#include "GameProgressState.h"
#include "Game.h"

GameProgressState::GameProgressState(Game* game)
    :mGame(game)
    ,mIsFirstBreak(false)
    ,mIsStageClear(false)
    ,mSceneState(SceneState::Opening)
{

}