#include "Boat.h"
#include "Planet.h"
#include "Stage.h"
#include "Game.h"

Boat::Boat(Game* game)
    : Actor(game)
    , mStartPlanet(0)
    , mDestPlanet(1)
    , mIsMoving(false)
    , mIsActive(true)
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
    // 移動中
    if (GetIsMoving())
    {   
        Planet* dest = mPlanets[mDestPlanet];
        glm::vec3 toDest = glm::normalize(dest->GetCenter() - mPos);
        mDestPos = dest->GetCenter() - toDest * (dest->GetRadius() + 3.0f);
        mTransitionTimer += deltaTime;

        // ボード移動がどれくらい進んだかの割合を更新
        float transitionDuration = 3.0f;
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