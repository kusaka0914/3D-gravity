#pragma once

#include <cstdint>

class Component {
public:
    Component(class Actor* owner, int updateOrder = 100);
    virtual ~Component();
    virtual void Update(float deltaTime);
    virtual void ProcessInput(const uint8_t* state);
    int GetUpdateOrder() const { return mUpdateOrder; }
    class Actor* GetOwner() const { return mOwner; }
protected:
    Actor* mOwner;
    int mUpdateOrder;
};