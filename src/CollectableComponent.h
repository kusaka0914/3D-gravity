#ifndef COLLECTABLECOMPONENT_H
#define COLLECTABLECOMPONENT_H

#include "Component.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class CollectableComponent : public Component {
public:
    CollectableComponent(class Actor* owner, int updateOrder = 100, bool isActive = false);
    void Update(float deltaTime) override;

    void SetIsActive(bool isActive) { mIsActive = isActive; }
    void SetIsObtained(bool isObtained) { mIsObtained = isObtained; }

    bool GetIsActive() const { return mIsActive; }
    bool GetIsObtained() const { return mIsObtained; }
private:
    bool mIsActive;
    bool mIsObtained;
};
#endif
