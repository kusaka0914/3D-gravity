#include "Enemy.h"
#include "Game.h"
#include "Planet.h"

Enemy::Enemy(Game* game)
    :Actor(game)
    ,mPos({0.0f, 0.0f, 0.0f})
    ,mCurrentPlanet(0)
    ,mHp(10.0f)
    ,mIsAlive(true)
    ,mDamageTimer(0.0f)
    ,mModelPath("enemy.obj")
    ,mScale(0.25f)
    ,mSpeed(2.0f)
    ,mAttack(20.0f)
    ,mStandByAttackTimer(-1.0f)
    ,mIsAttack(false)
    ,mSensing(6.0f)
{

}

void Enemy::UpdateActor(float deltaTime) {
    Actor::UpdateActor(deltaTime);
    std::vector<class Player*> players = GetGame()->GetPlayers();
    for(auto player : players) {
        glm::vec3 playerPos = player->GetPos();
        float distToPlayer = glm::length(playerPos - mPos);
        glm::vec3 vecToPlayer = glm::normalize(playerPos - mPos);

        int currentStageNum = GetGame()->GetCurrentStageNum();
        Stage* currentStage = GetGame()->GetStages()[currentStageNum].get();
        Planet* currentPlanet = currentStage->GetPlanets()[mCurrentPlanet].get();

        // 追跡
        if (distToPlayer <= mSensing && mDamageTimer <= 0.0f && !player->GetIsDamaged() && distToPlayer >= GetRadius())
        {
            mPos += vecToPlayer * mSpeed * deltaTime;
            float planetRadius = currentPlanet->GetRadius();
            glm::vec3 planetCenter = currentPlanet->GetCenter();
            mPos = planetCenter + glm::normalize(mPos - planetCenter) * planetRadius;
        }

        const float attackRangeMargin = 0.2f;
        bool inRangeOfPlayer = (distToPlayer <= GetRadius() + attackRangeMargin);
        // 待機
        if (!inRangeOfPlayer)
        {
            mStandByAttackTimer = -1.0f;
        }
        // 攻撃タイマー開始
        else if (mStandByAttackTimer == -1.0f)
        {
            if (!player->GetIsDamagePrev())
                mStandByAttackTimer = 2.0f;
        }
        // 攻撃準備
        if (mStandByAttackTimer >= 0.0f)
        {
            float prevStandBy = mStandByAttackTimer;
            mStandByAttackTimer -= deltaTime;
            const auto& seList = GetGame()->GetSEList();
            auto it = seList.find("attackPreSE");
            Mix_Chunk* attackPreSE = (it != seList.end()) ? it->second : nullptr;
            if (prevStandBy >= 0.5f && mStandByAttackTimer < 0.5f && attackPreSE)
            {
                Mix_PlayChannel(-1, attackPreSE, 0); // 攻撃直前SE
            }
        }
        // 攻撃
        if (mStandByAttackTimer <= 0.0f && inRangeOfPlayer)
        {
            mStandByAttackTimer = -1.0f;
            if (!player->GetIsDamagePrev())
            {
                player->SetHp(player->GetHp() - mAttack);
                player->SetDamageTimer(1.0f);
                player->SetIsDamaged(true);
                player->SetKnockBackFrom(mPos);
                if (player->GetHp() <= 0)
                {
                    player->SetHp(0);
                    player->SetPos(player->GetRestartPos());
                    player->SetPlanetIndex(player->GetRestartPlanetIndex());
                    player->SetVelocity(0.0f);
                    player->SetOnGround(true);
                }
            }
        }
    }
}