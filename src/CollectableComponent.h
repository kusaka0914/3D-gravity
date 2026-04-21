#pragma once

#include "Component.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class CollectableComponent : public Component {
public:
    CollectableComponent(class Actor* owner, int updateOrder = 100);
    void Update(float deltaTime) override;

    void SetIsObtained(bool isObtained) { mIsObtained = isObtained; }

    bool GetIsObtained() const { return mIsObtained; }
private:
    bool mIsObtained;
};
