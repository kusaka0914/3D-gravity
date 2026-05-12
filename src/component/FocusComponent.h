#pragma once

#include "component/Component.h"
#include <glm/glm.hpp>

class FocusComponent : public Component {
public:
    FocusComponent(class Actor* owner, int updateOrder = 100);
    void Update(float deltaTime) override;

    void SetIsFocused(bool isFocused) { mIsFocused = isFocused; }
    void SetFocusTimer(float focusTimer) { mFocusTimer = focusTimer; }

    bool GetIsFocused() const { return mIsFocused; }
    float GetFocusTimer() const { return mFocusTimer; }
    glm::mat4 GetFocusView();
    
private:
    bool mIsFocused;
    float mFocusTimer;
};
