#include "UIState.h"
#include "Game.h"

UIState::UIState(Game* game)
    :mGame(game)
    ,mTalkUIIndex(0)
    ,mIsBattleTutorialShown(false)
    ,mCurrentTalkWith(TalkWith::Opening)
    ,mCurrentTutorialKind(TutorialKind::None)
{

}