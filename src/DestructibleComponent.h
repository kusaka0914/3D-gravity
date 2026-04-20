#pragma once

#include "Component.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class DestructibleComponent : public Component {
public:
    DestructibleComponent(class Actor* owner, int updateOrder = 100, bool isActive = false);
    void Update(float deltaTime) override;

    void SetIsActive(bool isActive) { mIsActive = isActive; }
    void SetIsDestroyed(bool isDestroyed) { mIsDestroyed = isDestroyed; }

    bool GetIsActive() const { return mIsActive; }
    bool GetIsDestroyed() const { return mIsDestroyed; }
private:
    bool mIsActive;
    bool mIsDestroyed;
};
