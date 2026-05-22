#pragma once

#include <cstdint>

class Actor;

class Component {
public:
    Component(Actor* owner, int updateOrder = 100);
    virtual ~Component();

    virtual void Update(float deltaTime);

    virtual void ProcessInput(const uint8_t* state);

    int GetUpdateOrder() const { return mUpdateOrder; }
protected:
    Actor* mOwner;

    int mUpdateOrder;
};