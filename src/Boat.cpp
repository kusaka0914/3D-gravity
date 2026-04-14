#include "Boat.h"
#include "Planet.h"
#include "Stage.h"
#include "Game.h"

Boat::Boat(Game* game)
    : Actor(game)
    , mStartPlanet(0)
    , mDestPlanet(1)
    , mIsMoving(false)
    , mIsActive(false)
    , mTransitionTimer(0.0f)
    , mProgress(0.0f)
    , mPos({0.0f, 8.0f, 0.0f})
    , mStartPos(mPos)
    , mDestPos(0.0f)
    , mUpVec(0.0f, 1.0f, 0.0f)
{
    mCurrentPlanetNum = mStartPlanet;
}

void Boat::UpdateActor(float deltaTime)
{
    // 表示中
    if (GetIsActive()) {
        Planet* currentPlanet = mPlanets[mCurrentPlanetNum];
        // ボートを現在惑星の表面近くに配置
        float boatHeight = currentPlanet->GetRadius() - 0.15f;
        mPos = currentPlanet->GetCenter() + glm::normalize(glm::vec3(0.0f, -1.0f, 0.5f)) * boatHeight;
    }
    // 移動中
    if (GetIsMoving())
    {   
        Planet* from = mPlanets[mStartPlanet];
        Planet* dest = mPlanets[mDestPlanet];
        glm::vec3 toDest = glm::normalize(dest->GetCenter() - from->GetCenter());
        mDestPos = dest->GetCenter() - toDest * dest->GetRadius();
        mTransitionTimer = mTransitionTimer + deltaTime;

        // ボード移動がどれくらい進んだかの割合を更新
        float transitionDuration = 2.0f;
        mProgress = glm::min(1.0f, mTransitionTimer / transitionDuration);
        mProgress = mProgress * mProgress * (3.0f - 2.0f * mProgress);

        // ボートの位置を更新
        mPos = mStartPos + (mDestPos - mStartPos) * mProgress;

        // ボートから一番近い惑星を現在の惑星に更新
        float startDist = glm::length(mPos - mPlanets[mStartPlanet]->GetCenter());
        float destDist = glm::length(mPos - mPlanets[mDestPlanet]->GetCenter());
        mCurrentPlanetNum = (startDist < destDist) ? mStartPlanet : mDestPlanet;

        // 上ベクトルを更新
        mUpVec = glm::normalize(mPos - mPlanets[mCurrentPlanetNum]->GetCenter());
        if (mProgress >= 1.0f)
        {
            mPos = mDestPos;
            mIsMoving = false;
        }
    }
}