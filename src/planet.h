#pragma once

#include "Actor.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class Planet : public Actor {
public:
    enum class KeySpawnCondition {
        AllEnemiesDead,
        AllBoatPartsCollected,
        None
    };

    enum class PlanetType {
        Normal,
        Sphere
    };

    Planet(class Game* game);
    void Initialize() override;
    void UpdateActor(float deltaTime) override;
    void AddEnemy(class Enemy* enemy) { mEnemies.emplace_back(enemy); }
    void AddBoat(class Boat* boat) { mBoats.emplace_back(boat); }
    void AddBoatParts(class BoatParts* boatParts) { mBoatParts.emplace_back(boatParts); }
    void AddCrystals(class Crystal* crystal) { mCrystals.emplace_back(crystal); }
    void AddEnemyMesh(std::string modelPath, std::vector<LoadedMesh> meshes) { mEnemyMeshesByPath[modelPath] = meshes; }

    void RemoveAllEnemy() { if (!mEnemies.empty()) for(int i = 0; i < mEnemies.size(); i++) mEnemies.pop_back(); }
    void RemoveAllBoat() { if (!mBoats.empty())  for(int i = 0; i < mBoats.size(); i++) mBoats.pop_back(); }
    void RemoveAllBoatParts() { if (!mBoatParts.empty())  for(int i = 0; i < mBoatParts.size(); i++) mBoatParts.pop_back(); }
    void RemoveAllCrystals() { if (!mCrystals.empty())  for(int i = 0; i < mCrystals.size(); i++) mCrystals.pop_back(); }
    void RemoveKey() { mKey = nullptr; }
    void RemoveStar() { mStar = nullptr; }

    void SetCurrentStage(class Stage* currentStage) { mCurrentStage = currentStage; }
    void buildSphereMesh(unsigned int segmentsLat, unsigned int segmentsLong, float radius,
        std::vector<float>& outVertices, std::vector<unsigned int>& outIndices);
    void SetStageNum(int stageNum) { mStageNum = stageNum; }
    void SetCenter(glm::vec3 center) { mCenter = center; }
    void SetRadius(float radius) { mRadius = radius; }
    void SetColor(glm::vec3 color) { mColor = color; }
    void SetModelPath(std::string modelPath) { mModelPath = modelPath; }
    void SetKey(class Key* key) { mKey = key; }
    void SetKeySpawnCondition(std::string keySpawnCondition) { 
        if(keySpawnCondition == "AllEnemiesDead") {
            mKeySpawnCondition = KeySpawnCondition::AllEnemiesDead;
        }else if(keySpawnCondition == "AllBoatPartsCollected") {
            mKeySpawnCondition = KeySpawnCondition::AllBoatPartsCollected;
        }else {
            mKeySpawnCondition = KeySpawnCondition::None;
        }
    }
    void SetPlanetType(std::string planetType) { 
        if(planetType == "Normal") {
            mPlanetType = PlanetType::Normal;
        }else if (planetType == "Sphere") {
            mPlanetType = PlanetType::Sphere;
        }
    }

    class Stage* GetCurrentStage() const { return mCurrentStage; }
    int GetStageNum() const { return mStageNum; }
    const glm::vec3& GetCenter() const { return mCenter; }
    float GetRadius() const override { return mRadius; }
    const glm::vec3& GetColor() const { return mColor; }
    const std::string& GetModelPath() const { return mModelPath; }
    const std::vector<class Enemy*>& GetEnemies() const { return mEnemies; }
    const std::vector<class Boat*>& GetBoats() const { return mBoats; }
    const std::vector<class BoatParts*>& GetBoatParts() const { return mBoatParts; }
    const std::vector<class Crystal*>& GetCrystals() const { return mCrystals; }
    class Key* GetKey() const { return mKey; }
    class Star* GetStar() const { return mStar; }
    PlanetType GetPlanetType() const { return mPlanetType; }
    const std::unordered_map<std::string, std::vector<LoadedMesh>>& GetEnemyMeshesByPath() const { return mEnemyMeshesByPath; }

private:
    class Stage* mCurrentStage;
    int mStageNum;
    glm::vec3 mCenter;
    float mRadius;
    glm::vec3 mColor;
    std::string mModelPath;
    std::vector<class Enemy*> mEnemies;
    std::vector<class Boat*> mBoats;
    std::vector<class BoatParts*> mBoatParts;
    std::vector<class Crystal*> mCrystals;
    class Key* mKey;
    class Star* mStar;
    bool mIsAllEnemiesDead;
    bool mIsAllBoatPartsCollected;

    KeySpawnCondition mKeySpawnCondition;
    PlanetType mPlanetType;

    std::unordered_map<std::string, std::vector<LoadedMesh>> mEnemyMeshesByPath;
};