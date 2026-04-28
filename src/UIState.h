class UIState {
public:
    UIState(class Game* game);
    void SetIsUIActive(bool isUIActive) { mIsUIActive = isUIActive; }
    void SetIsTitleActive(bool isTitleActive) { mIsTitleActive = isTitleActive; }
    void SetIsTutorialActive(bool isTutorialActive) { mIsTutorialActive = isTutorialActive; }
    void SetIsCrystalTutorialActive(bool isCrystalTutorialActive) { mIsCrystalTutorialActive = isCrystalTutorialActive; }
    void SetIsBattleTutorialActive(bool isBattleTutorialActive) { mIsBattleTutorialActive = isBattleTutorialActive; }
    void SetIsBattleTutorialShown(bool isBattleTutorialShown) { mIsBattleTutorialShown = isBattleTutorialShown; }
    void SetIsBreakTutorialActive(bool isBreakTutorialActive) { mIsBreakTutorialActive = isBreakTutorialActive; }

    bool GetIsUIActive() const { return mIsUIActive; }
    bool GetIsTitleActive() const { return mIsTitleActive; }
    bool GetIsTutorialActive() const { return mIsTutorialActive; }
    bool GetIsCrystalTutorialActive() const { return mIsCrystalTutorialActive; }
    bool GetIsBattleTutorialActive() const { return mIsBattleTutorialActive; }
    bool GetIsBattleTutorialShown() const { return mIsBattleTutorialShown; }
    bool GetIsBreakTutorialActive() const { return mIsBreakTutorialActive; }
private:
    Game* mGame;
    bool mIsUIActive;
    bool mIsTitleActive;
    bool mIsTutorialActive;
    bool mIsCrystalTutorialActive;
    bool mIsBattleTutorialActive;
    bool mIsBattleTutorialShown;
    bool mIsBreakTutorialActive;
};