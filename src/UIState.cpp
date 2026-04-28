#include "UIState.h"
#include "Game.h"

UIState::UIState(Game* game)
    :mGame(game)
    ,mIsUIActive(false)
    ,mIsTitleActive(true)
    ,mIsTutorialActive(false)
    ,mIsCrystalTutorialActive(false)
    ,mIsBattleTutorialActive(false)
    ,mIsBattleTutorialShown(false)
    ,mIsBreakTutorialActive(false)
{

}