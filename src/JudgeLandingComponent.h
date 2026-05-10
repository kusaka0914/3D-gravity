#pragma once

#include "Component.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class JudgeLandingComponent : public Component {
public:
    JudgeLandingComponent(class Actor* owner, int updateOrder = 100);
    void Update(float deltaTime) override;

private:
    void JudgeLanding();

private:
    
};
