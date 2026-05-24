#pragma once

#include <string>

class Game;

class UIState {
public:
    enum class TalkWith {
        None,
        Opening,
        Mother,
        Doctor,
        NPC
    };

    enum class TutorialKind {
        None,
        Battle,
        Break,
        SpecialAttack
    };

    UIState(Game* game);
    void IncTalkUIIndex() { mTalkUIIndex++; }
    void StartTalkWith(TalkWith talkWith);
    void FinishTutorial();
    void FinishTalkWith();
    void OnFadeIn();

    void SetIsBattleTutorialShown(bool isBattleTutorialShown) { mIsBattleTutorialShown = isBattleTutorialShown; }
    void SetIsSpecialAttackTutorialShown(bool isSpecialAttackTutorialShown) { mIsSpecialAttackTutorialShown = isSpecialAttackTutorialShown; }

    void SetTalkUIIndex(int talkUIIndex) { mTalkUIIndex = talkUIIndex; }
    
    void SetCurrentTalkWith(TalkWith currentTalkWith) { mCurrentTalkWith = currentTalkWith; }
    void SetCurrentTutorialKind(TutorialKind currentTutorialKind) { mCurrentTutorialKind = currentTutorialKind; }

    bool GetIsBattleTutorialShown() const { return mIsBattleTutorialShown; }
    bool GetIsSpecialAttackTutorialShown() const { return mIsSpecialAttackTutorialShown; }

    int GetTalkUIIndex() const { return mTalkUIIndex; }
    
    TalkWith GetCurrentTalkWith() const { return mCurrentTalkWith; }
    TutorialKind GetCurrentTutorialKind() const { return mCurrentTutorialKind; }

private:
    bool mIsBattleTutorialShown;
    bool mIsSpecialAttackTutorialShown;

    int mTalkUIIndex;

    TalkWith mCurrentTalkWith;
    TutorialKind mCurrentTutorialKind;

    Game* mGame;
};