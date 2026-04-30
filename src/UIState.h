#include <string>

class UIState {
public:
    enum class TalkWith {
        Opening,
        Mother,
        Doctor,
        NPC
    };
    enum class TutorialKind {
        None,
        BoatParts,
        Crystal,
        Battle,
        Break
    };
    UIState(class Game* game);
    void SetTalkUIIndex(int talkUIIndex) { mTalkUIIndex = talkUIIndex; }
    void SetIsTutorialShown(bool isTutorialShown) { mIsTutorialShown = isTutorialShown; }
    void SetIsBattleTutorialShown(bool isBattleTutorialShown) { mIsBattleTutorialShown = isBattleTutorialShown; }
    void IncTalkUIIndex() { mTalkUIIndex++; }
    void SetTalkWith(std::string talkWith) {
        if (talkWith == "Mother") {
            mTalkWith = TalkWith::Mother;
        } else if (talkWith == "Doctor") {
            mTalkWith = TalkWith::Doctor;
        } else if (talkWith == "NPC") {
            mTalkWith = TalkWith::NPC;
        } else {
            mTalkWith = TalkWith::Opening;
        }
    }
    void SetCurrentTutorialKind(std::string currentTutorialKind) {
        if (currentTutorialKind == "BoatParts") {
            mCurrentTutorialKind = TutorialKind::BoatParts;
        } else if (currentTutorialKind == "Crystal") {
            mCurrentTutorialKind = TutorialKind::Crystal;
        } else if (currentTutorialKind == "Battle") {
            mCurrentTutorialKind = TutorialKind::Battle;
        } else if (currentTutorialKind == "Break") {
            mCurrentTutorialKind = TutorialKind::Break;
        } else {
            mCurrentTutorialKind = TutorialKind::None;
        }
    }

    int GetTalkUIIndex() const { return mTalkUIIndex; }
    bool GetIsTutorialShown() const { return mIsTutorialShown; }
    bool GetIsBattleTutorialShown() const { return mIsBattleTutorialShown; }
    TalkWith GetTalkWith() const { return mTalkWith; }
    TutorialKind GetCurrentTutorialKind() const { return mCurrentTutorialKind; }

private:
    Game* mGame;

    TalkWith mTalkWith;
    TutorialKind mCurrentTutorialKind;

    bool mIsTutorialShown;
    bool mIsBattleTutorialShown;

    int mTalkUIIndex;
};