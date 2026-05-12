#pragma once

#include "component/Component.h"

class Actor;

class JudgeLandingComponent : public Component {
public:
    JudgeLandingComponent(Actor* owner, int updateOrder = 100);
    void Update(float deltaTime) override;

private:
    void JudgeLanding();

private:
    
};
