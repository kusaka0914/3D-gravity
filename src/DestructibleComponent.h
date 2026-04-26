#pragma once

#include "Component.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class DestructibleComponent : public Component {
public:
    DestructibleComponent(class Actor* owner, int updateOrder = 100);
    void Update(float deltaTime) override;

    void SetIsDestroyed(bool isDestroyed) { mIsDestroyed = isDestroyed; }
    void SetDestroyHp(int destroyHp) { mDestroyHp = destroyHp; }

    bool GetIsDestroyed() const { return mIsDestroyed; }
    // int GetDestroyCount() const { return mDestroyCount; }
private:
    bool mIsDestroyed;
    bool mIsAttackedPrev;
    float mDestroyHp;
};
