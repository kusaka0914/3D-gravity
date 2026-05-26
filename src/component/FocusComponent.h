#pragma once

#include "component/Component.h"

class Actor;

class FocusComponent : public Component {
public:
    FocusComponent(Actor* owner, int updateOrder = 100);
    void Update(float deltaTime) override;
    void StartFocus();

    bool GetIsFocused() const { return mIsFocused; }
    float GetFocusTimer() const { return mFocusTimer; }

private:
    
private:
    bool mIsFocused;
    float mFocusTimer;
};
