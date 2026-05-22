#include "UIState.h"
#include "Game.h"

UIState::UIState(Game* game)
    :mGame(game)
    , mTalkUIIndex(0)
    , mIsBattleTutorialShown(false)
    , mCurrentTalkWith(TalkWith::Opening)
    , mCurrentTutorialKind(TutorialKind::None)
{

}

void UIState::StartTalkWith(TalkWith talkWith) {
    mTalkUIIndex = 0;
    mCurrentTalkWith = talkWith;
}

void UIState::FinishTutorial() {
    mTalkUIIndex = 0;
    mCurrentTutorialKind = TutorialKind::None;
}

void UIState::FinishTalkWith() {
    mTalkUIIndex = 0;
    mCurrentTalkWith = TalkWith::None;
}

void UIState::OnFadeIn() {
    mTalkUIIndex = -1;
}