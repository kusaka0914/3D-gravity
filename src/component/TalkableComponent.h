#pragma once

#include "component/Component.h"
#include <string>
#include <vector>

class Actor;
class Player;

class TalkableComponent : public Component {
public:
    TalkableComponent(Actor* owner, int updateOrder = 100);
    void Update(float deltaTime) override;

    void AddTalkTexts(const std::string& talkTexts) { mTalkTexts.emplace_back(talkTexts); }

    bool GetIsTalkable() const { return mIsTalkable; }
    const std::vector<std::string>& GetTalkTexts() const { return mTalkTexts; }

private:
    bool IsPlayerInTalkableRange(Player* player);

private:
    bool mIsTalkable;
    std::vector<std::string> mTalkTexts;
};
