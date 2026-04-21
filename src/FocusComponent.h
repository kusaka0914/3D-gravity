#pragma once

#include "Component.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class FocusComponent : public Component {
public:
    FocusComponent(class Actor* owner, int updateOrder = 100);
    void Update(float deltaTime) override;

    void SetFocusTimer(float focusTimer) { mFocusTimer = focusTimer; }

    float GetFocusTimer() const { return mFocusTimer; }
    glm::mat4 GetFocusView();
private:
    float mFocusTimer;
};
