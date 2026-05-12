#pragma once

#include "Component.h"

class Actor;

class CollectableComponent : public Component {
public:
    CollectableComponent(Actor* owner, int updateOrder = 100);

    void Update(float deltaTime) override;

    bool GetIsObtained() const { return mIsObtained; }

private:
    void TryCollect();

private:
    bool mIsObtained;
};
