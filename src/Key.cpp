#include "Key.h"
#include "Planet.h"
#include "Stage.h"
#include "Game.h"
#include "Player.h"
#include "Boat.h"

Key::Key(Game* game)
    :Actor(game)
{

}

void Key::UpdateActor(float deltaTime)
{
    std::vector<class Player*> players = GetGame()->GetPlayers();
    if (GetIsActive()) {
        for (auto player : players) {
            mCurrentPlanet = player->GetCurrentPlanetNum();
            // mPos = lastDefeatedEnemyPos; // 最後に倒した敵の場所に鍵を出す

            // 鍵に触れたら取得して消す＆ボートを出現させる
            float distToKey = glm::length(player->GetPos() - mPos);
            const float keyPickupRadius = 1.2f;
            if (distToKey < keyPickupRadius)
            {
                mIsActive = false;
                mIsObtained = true;

                int currentStageNum = GetGame()->GetCurrentStageNum();
                Stage* currentStage = GetGame()->GetStages()[currentStageNum];
                std::vector<Planet*> planets = currentStage->GetPlanets();
                std::vector<Boat*> boats = planets[mCurrentPlanet]->GetBoats();
                for (auto boat : boats) {
                    boat->SetIsActive(true);
                }
            }
        }
    }
}