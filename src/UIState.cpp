#include "UIState.h"
#include "Game.h"

UIState::UIState(Game* game)
    :mGame(game)
    ,mIsUIActive(false)
    ,mIsOpeningUIActive(false)
    ,mOpeningUIIndex(0)
    ,mTalkUIIndex(0)
    ,mIsTutorialActive(false)
    ,mIsCrystalTutorialActive(false)
    ,mIsBattleTutorialActive(false)
    ,mIsBattleTutorialShown(false)
    ,mIsBreakTutorialActive(false)
{

}