#include "Planet.h"
#include "Game.h"
#include "Enemy.h"
#include "Boat.h"
#include "Key.h"
#include "Stage.h"
#include "Star.h"
#include "BoatParts.h"
#include <cmath>

Planet::Planet(Game* game)
    : Actor(game)
    , mCenter(0.0f)
    , mRadius(8.0f)
    , mColor(1.0f)
    , mModelPath("planet.obj")
    , mKey(nullptr)
{
    
}

void Planet::Initialize()
{
    auto starUnique = std::make_unique<Star>(GetGame());
    Star* star = starUnique.get();
    GetGame()->AddActor(std::move(starUnique));
    mStar = star;
}

void Planet::UpdateActor(float deltaTime) {
    Actor::UpdateActor(deltaTime);
    
    switch(mKeySpawnCondition) {
        case KeySpawnCondition::AllEnemiesDead: {
            // 今いる惑星の敵を全て倒したら鍵を出現させる
            bool isAllEnemiesDead = true;
            for (auto enemy : mEnemies) {
                if (enemy->GetIsAlive())
                {
                    isAllEnemiesDead = false;
                    break;
                }
            }
            if (isAllEnemiesDead && !mKey->GetCollectableComponent()->GetIsActive() && !mKey->GetCollectableComponent()->GetIsObtained())
            {
                mKey->GetCollectableComponent()->SetIsActive(true);
            }
            break;
        }
        case KeySpawnCondition::AllBoatPartsCollected: {
            bool isAllBoatPartsCollected = true;
            for (auto parts : mBoatParts) {
                if (parts->GetCollectableComponent()->GetIsActive())
                {
                    isAllBoatPartsCollected = false;
                    break;
                }
            }
            if (isAllBoatPartsCollected && !mKey->GetCollectableComponent()->GetIsActive() && !mKey->GetCollectableComponent()->GetIsObtained())
            {
                mKey->GetCollectableComponent()->SetIsActive(true);
            }
            break;
        }
    }
}

void Planet::buildSphereMesh(unsigned int segmentsLat, unsigned int segmentsLong, float radius,
    std::vector<float>& outVertices, std::vector<unsigned int>& outIndices) {
    outVertices.clear();
    outIndices.clear();

    // 球全体の頂点の座標を計算して格納
    for (unsigned int i = 0; i <= segmentsLat; ++i) {
        float phi = i * 3.14159265f / static_cast<float>(segmentsLat);
        for (unsigned int j = 0; j <= segmentsLong; ++j) {
            float theta = j * 2.0f * 3.14159265f / static_cast<float>(segmentsLong);
            // 公式を用いて各座標を計算
            float x = radius * std::sin(phi) * std::cos(theta);
            float y = radius * std::cos(phi);
            float z = radius * std::sin(phi) * std::sin(theta);
            outVertices.emplace_back(x);
            outVertices.emplace_back(y);
            outVertices.emplace_back(z);
        }
    }

    // インデックスを格納
    for (unsigned int i = 0; i < segmentsLat; ++i) {
        for (unsigned int j = 0; j < segmentsLong; ++j) {
            // 左上、右上、左下、右下のインデックスを計算
            // 各段にはsegmentsLong + 1個あるので、その左、右、下の段の左、右は以下のように計算
            // 例: i=0:  0, 1, 2
            //     i=1:  3, 4, 5
            //     i=2:  6, 7, 8
            unsigned int i0 = i * (segmentsLong + 1) + j;
            unsigned int i1 = i0 + 1;
            unsigned int i2 = i0 + (segmentsLong + 1);
            unsigned int i3 = i2 + 1;
            // 四角形を2つの三角形に分ける
            outIndices.emplace_back(i0);
            outIndices.emplace_back(i2);
            outIndices.emplace_back(i1);
            outIndices.emplace_back(i1);
            outIndices.emplace_back(i2);
            outIndices.emplace_back(i3);
        }
    }
}
