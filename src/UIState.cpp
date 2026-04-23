#include "UIState.h"
#include "Game.h"

UIState::UIState(Game* game)
    :mGame(game)
    ,mIsUIActive(false)
    ,mIsTutorialActive(true)
    ,mIsCrystalTutorialActive(false)
{

}