class GameProgressState {
public:
    GameProgressState(class Game* game);
    void SetIsFirstBreak(bool isFirstBreak) { mIsFirstBreak = isFirstBreak; }
    void SetIsStageClear(bool isStageClear) { mIsStageClear = isStageClear; }
    // void SetIsTutorialActive(bool isTutorialActive) { mIsTutorialActive = isTutorialActive; }
    // void SetIsCrystalTutorialActive(bool isCrystalTutorialActive) { mIsCrystalTutorialActive = isCrystalTutorialActive; }
    // void SetIsBattleTutorialActive(bool isBattleTutorialActive) { mIsBattleTutorialActive = isBattleTutorialActive; }
    // void SetIsBattleTutorialShown(bool isBattleTutorialShown) { mIsBattleTutorialShown = isBattleTutorialShown; }

    bool GetIsFirstBreak() const { return mIsFirstBreak; }
    bool GetIsStageClear() const { return mIsStageClear; }
    // bool GetIsTutorialActive() const { return mIsTutorialActive; }
    // bool GetIsCrystalTutorialActive() const { return mIsCrystalTutorialActive; }
    // bool GetIsBattleTutorialActive() const { return mIsBattleTutorialActive; }
    // bool GetIsBattleTutorialShown() const { return mIsBattleTutorialShown; }
private:
    Game* mGame;
    bool mIsFirstBreak;
    bool mIsStageClear;
    // bool mIsTutorialActive;
    // bool mIsCrystalTutorialActive;
    // bool mIsBattleTutorialActive;
    // bool mIsBattleTutorialShown;
};