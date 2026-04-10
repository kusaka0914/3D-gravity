#include "Boat.h"
#include "Planet.h"
#include "Stage.h"
#include "Game.h"

Boat::Boat(Game* game)
    :Actor(game)
{

}

void Boat::UpdateActor(float deltaTime)
{
    if (GetIsActive())
    {   
        float transitionTimer = GetTransitionTimer();
        float updateTransitionTimer = transitionTimer + deltaTime;
        SetTransitionTimer(updateTransitionTimer);

        // ボード移動がどれくらい進んだかの割合を更新
        float transitionDuration = GetTransitionDuration();
        float updateProgress = glm::min(1.0f, transitionTimer / transitionDuration);
        updateProgress = updateProgress * updateProgress * (3.0f - 2.0f * updateProgress);
        SetProgress(updateProgress);

        // ボートの位置を更新
        glm::vec3 startPos = GetStartPos();
        glm::vec3 updatePos = GetStartPos() + (GetDestPos() - GetStartPos()) * mProgress;
        SetPos(updatePos);

        int currentStageNum = GetGame()->GetCurrentStageNum();
        Stage* currentStage = GetGame()->GetStages()[currentStageNum].get();
        std::vector<std::unique_ptr<Planet>> planets = currentStage->GetPlanets();

        // ボートから一番近い惑星を現在の惑星に更新
        float startDist = glm::length(mPos - planets[mStartPlanet].get()->GetCenter());
        float destDist = glm::length(mPos - planets[mDestPlanet].get()->GetCenter());
        int nearestPlanetId = (startDist < destDist) ? startDist : destDist;
        SetCurrentPlanet(nearestPlanetId);

        // 上ベクトルを更新
        glm::vec3 updateUpVec = glm::normalize(mPos - planets[mCurrentPlanet].get()->GetCenter());
        SetUpVec(UpdateUpVec);

        players[0].pos = boatPos + boatUp * playerHeightAboveBoat;
        // 到着処理
        if (t >= 1.0f)
        {
            players[0].planetIndex = boatDestinationPlanetIndex;
            boatPos = boatTransitionEnd;
            players[0].pos = boatTransitionEnd;
            players[0].onGround = true;
            players[0].velocity = glm::vec3(0.0f);
            restartPos = boatTransitionEnd;
            restartPlanetIndex = boatDestinationPlanetIndex;
            boatTransitionActive = false;
            if (bulletGhost)
            {
                btTransform t;
                t.setIdentity();
                t.setOrigin(btVector3(players[0].pos.x, players[0].pos.y, players[0].pos.z));
                bulletGhost->setWorldTransform(t);
            }
        }
    }
}