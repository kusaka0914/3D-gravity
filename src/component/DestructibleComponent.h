#pragma once

#include "Component.h"

class Actor;
class Player;

class DestructibleComponent : public Component {
public:
    DestructibleComponent(Actor* owner, int updateOrder = 100);
    void Update(float deltaTime) override;

    void SetDestroyHp(int destroyHp) { mDestroyHp = destroyHp; }

    bool GetIsDestroyed() const { return mIsDestroyed; }

private:
    void TryApplyDamage();
    void ApplyDamage(const float attack);

    bool IsPlayerInHitRange(Player* player) const;

private:
    bool mIsDestroyed;
    bool mIsHitCurrentAttack;

    float mDestroyHp;
};
