class UIState {
public:
    UIState(class Game* game);
    void SetIsUIActive(bool isUIActive) { mIsUIActive = isUIActive; }
    void SetIsOpeningUIActive(bool isOpeningUIActive) { mIsOpeningUIActive = isOpeningUIActive; }
    void SetIsTutorialActive(bool isTutorialActive) { mIsTutorialActive = isTutorialActive; }
    void SetIsCrystalTutorialActive(bool isCrystalTutorialActive) { mIsCrystalTutorialActive = isCrystalTutorialActive; }
    void SetIsBattleTutorialActive(bool isBattleTutorialActive) { mIsBattleTutorialActive = isBattleTutorialActive; }
    void SetIsBattleTutorialShown(bool isBattleTutorialShown) { mIsBattleTutorialShown = isBattleTutorialShown; }
    void SetIsBreakTutorialActive(bool isBreakTutorialActive) { mIsBreakTutorialActive = isBreakTutorialActive; }
    void IncOpeningUIIndex() { mOpeningUIIndex++; }
    void IncTalkUIIndex() { mTalkUIIndex++; }

    bool GetIsUIActive() const { return mIsUIActive; }
    bool GetIsOpeningUIActive() const { return mIsOpeningUIActive; }
    int GetOpeningUIIndex() const { return mOpeningUIIndex; }
    int GetTalkUIIndex() const { return mTalkUIIndex; }
    bool GetIsTutorialActive() const { return mIsTutorialActive; }
    bool GetIsCrystalTutorialActive() const { return mIsCrystalTutorialActive; }
    bool GetIsBattleTutorialActive() const { return mIsBattleTutorialActive; }
    bool GetIsBattleTutorialShown() const { return mIsBattleTutorialShown; }
    bool GetIsBreakTutorialActive() const { return mIsBreakTutorialActive; }
private:
    Game* mGame;
    bool mIsUIActive;
    bool mIsOpeningUIActive;
    int mOpeningUIIndex;
    int mTalkUIIndex;
    bool mIsTutorialActive;
    bool mIsCrystalTutorialActive;
    bool mIsBattleTutorialActive;
    bool mIsBattleTutorialShown;
    bool mIsBreakTutorialActive;
};