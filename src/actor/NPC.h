#pragma once

#include "actor/CharacterActor.h"
#include <glm/glm.hpp>

class Game;
class Player;

class NPC : public CharacterActor {
public:
    NPC(Game* game);
    ~NPC();

    void UpdateActor(float deltaTime) override;

    void AddTalkTexts(const std::string& talkTexts) { mTalkTexts.emplace_back(talkTexts); }

    void SetName(std::string name) { mName = name; }

    bool GetIsTalkable() const { return mIsTalkable; }
    std::string GetName() const { return mName; }
    const std::vector<std::string>& GetTalkTexts() const { return mTalkTexts; }

private:
    void LookNearestPlayer();
    void CheckTalkable();

    Player* FindNearestPlayer();
    bool IsPlayerInTalkableRange(Player* player) const;

private:
    bool mIsTalkable;
    std::string mName;
    std::vector<std::string> mTalkTexts;
};