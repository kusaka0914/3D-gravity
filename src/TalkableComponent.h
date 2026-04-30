#pragma once

#include "Component.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class TalkableComponent : public Component {
public:
    TalkableComponent(class Actor* owner, int updateOrder = 100);
    void Update(float deltaTime) override;

    void SetIsTalkable(bool isTalkable) { mIsTalkable = isTalkable; }
    void AddTalkTexts(const std::string& talkTexts) { mTalkTexts.emplace_back(talkTexts); }

    bool GetIsTalkable() const { return mIsTalkable; }
    const std::vector<std::string>& GetTalkTexts() const { return mTalkTexts; }

private:
    bool mIsTalkable;
    std::vector<std::string> mTalkTexts;
};
