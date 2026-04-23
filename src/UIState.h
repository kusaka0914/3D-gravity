class UIState {
public:
    UIState(class Game* game);
    void SetIsUIActive(bool isUIActive) { mIsUIActive = isUIActive; }
    void SetIsTutorialActive(bool isTutorialActive) { mIsTutorialActive = isTutorialActive; }
    void SetIsCrystalTutorialActive(bool isCrystalTutorialActive) { mIsCrystalTutorialActive = isCrystalTutorialActive; }

    bool GetIsUIActive() const { return mIsUIActive; }
    bool GetIsTutorialActive() const { return mIsTutorialActive; }
    bool GetIsCrystalTutorialActive() const { return mIsCrystalTutorialActive; }
private:
    Game* mGame;
    bool mIsUIActive;
    bool mIsTutorialActive;
    bool mIsCrystalTutorialActive;
};