#include "Boat.h"
#include "Planet.h"
#include "Stage.h"
#include "Game.h"

Boat::Boat(Game* game)
    :Actor(game)
{
    int currentStageNum = GetGame()->GetCurrentStageNum();
    Stage* currentStage = GetGame()->GetStages()[currentStageNum];
    mPlanets = currentStage->GetPlanets();
}

void Boat::UpdateActor(float deltaTime)
{
    // 表示中
    if (GetIsActive()) {
        // int curIdx = GetCurrentPlanet();
        // boatSpawnPlanetIndex = curIdx;
        // // 現在惑星から一番近い別惑星を到着先に
        // float nearestDist = 1e30f;
        // for (size_t i = 0; i < mPlanets.size(); i++)
        // {
        //     if (static_cast<int>(i) == curIdx)
        //         continue;
        //     float d = glm::length(mPlanets[i].center - mPlanets[curIdx].center);
        //     if (d < nearestDist)
        //     {
        //         nearestDist = d;
        //         boatDestinationPlanetIndex = static_cast<int>(i);
        //     }
        // }
        // // ボートを現在惑星の表面近くに配置
        // float boatHeight = mPlanets[curIdx].radius - 0.15f;
        // boatPos = mPlanets[curIdx].center + glm::normalize(glm::vec3(0.0f, -1.0f, 0.5f)) * boatHeight;
    }
    // 移動中
    if (GetIsMoving())
    {   
        mTransitionTimer = mTransitionTimer + deltaTime;

        // ボード移動がどれくらい進んだかの割合を更新
        mProgress = glm::min(1.0f, mTransitionTimer / mTransitionDuration);
        mProgress = mProgress * mProgress * (3.0f - 2.0f * mProgress);

        // ボートの位置を更新
        mPos = mStartPos + (mDestPos - mStartPos) * mProgress;

        // ボートから一番近い惑星を現在の惑星に更新
        float startDist = glm::length(mPos - mPlanets[mStartPlanet]->GetCenter());
        float destDist = glm::length(mPos - mPlanets[mDestPlanet]->GetCenter());
        mCurrentPlanet = (startDist < destDist) ? startDist : destDist;

        // 上ベクトルを更新
        mUpVec = glm::normalize(mPos - mPlanets[mCurrentPlanet]->GetCenter());
        if (mProgress >= 1.0f)
        {
            mPos = mDestPos;
            mIsMoving = false;
        }
    }
}