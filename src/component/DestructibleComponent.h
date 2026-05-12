#pragma once

#include "Component.h"

class Actor;

class DestructibleComponent : public Component {
public:
    DestructibleComponent(Actor* owner, int updateOrder = 100);
    void Update(float deltaTime) override;

    void SetIsDestroyed(bool isDestroyed) { mIsDestroyed = isDestroyed; }
    void SetDestroyHp(int destroyHp) { mDestroyHp = destroyHp; }

    bool GetIsDestroyed() const { return mIsDestroyed; }

private:
    void TryDestroyed();
    void OnDamaged(const float attack);
    void OnDestroyed();

private:
    bool mIsDestroyed;
    bool mIsAttackedPrev;

    float mDestroyHp;
};
