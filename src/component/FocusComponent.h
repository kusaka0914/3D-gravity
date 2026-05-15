#pragma once

#include "component/Component.h"
#include <glm/glm.hpp>

class FocusComponent : public Component {
public:
    FocusComponent(class Actor* owner, int updateOrder = 100);
    void Update(float deltaTime) override;
    void StartFocus();

    bool GetIsFocused() const { return mIsFocused; }
    float GetFocusTimer() const { return mFocusTimer; }
    glm::mat4 GetFocusView();

private:
    
private:
    bool mIsFocused;
    float mFocusTimer;
};
