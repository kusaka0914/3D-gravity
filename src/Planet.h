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

    enum class PlanetShape {
        Normal,
        Sphere
    };

    Planet(class Game* game);
    void Initialize() override;
    void UpdateActor(float deltaTime) override;
    void AddEnemy(class Enemy* enemy) { mEnemies.emplace_back(enemy); }
    void AddBoat(class Boat* boat) { mBoats.emplace_back(boat); }
    void AddBoatParts(class BoatParts* boatParts) { mBoatParts.emplace_back(boatParts); }
    void AddCrystal(class Crystal* crystal) { mCrystals.emplace_back(crystal); }
    void AddNPC(class NPC* NPC) { mNPCs.emplace_back(NPC); }
    void AddPlatform(class Platform* platform) { mPlatforms.emplace_back(platform); }
    void AddEnemyMesh(std::string modelPath, std::vector<struct LoadedMesh>* meshes) { mEnemyMeshesByPath[modelPath] = meshes; }

    void RemoveAllEnemy() { mEnemies.clear(); }
    void RemoveAllBoat() { mBoats.clear(); }
    void RemoveAllBoatParts() { mBoatParts.clear(); }
    void RemoveAllCrystals() { mCrystals.clear(); }
    void RemoveAllNPCs() { mNPCs.clear(); }
    void RemoveAllPlatforms() { mPlatforms.clear(); }
    void RemoveKey() { mKey = nullptr; }
    void RemoveStar() { mStar = nullptr; }

    void SetCurrentStage(class Stage* currentStage) { mCurrentStage = currentStage; }
    void buildSphereMesh(unsigned int segmentsLat, unsigned int segmentsLong, float radius,
        std::vector<float>& outVertices, std::vector<unsigned int>& outIndices);
    void SetStageNum(int stageNum) { mStageNum = stageNum; }
    void SetRadius(float radius) { mRadius = radius; }
    void SetColor(glm::vec4 color) { mColor = color; }
    void SetKey(class Key* key) { mKey = key; }
    void SetStar(class Star* star) { mStar = star; }
    void SetMeshes(std::vector<struct LoadedMesh>* meshes) { mMeshes = meshes; }
    void SetKeySpawnCondition(std::string keySpawnCondition) { 
        if(keySpawnCondition == "AllEnemiesDead") {
            mKeySpawnCondition = KeySpawnCondition::AllEnemiesDead;
        }else if(keySpawnCondition == "AllBoatPartsCollected") {
            mKeySpawnCondition = KeySpawnCondition::AllBoatPartsCollected;
        }else {
            mKeySpawnCondition = KeySpawnCondition::None;
        }
    }
    void SetPlanetShape(std::string PlanetShape) { 
        if(PlanetShape == "Normal") {
            mPlanetShape = PlanetShape::Normal;
        }else if (PlanetShape == "Sphere") {
            mPlanetShape = PlanetShape::Sphere;
        }
    }

    class Stage* GetCurrentStage() const { return mCurrentStage; }
    int GetStageNum() const { return mStageNum; }
    float GetRadius() const { return mRadius; }
    const glm::vec4& GetColor() const { return mColor; }
    const std::vector<class Enemy*>& GetEnemies() const { return mEnemies; }
    const std::vector<class Boat*>& GetBoats() const { return mBoats; }
    const std::vector<class BoatParts*>& GetBoatParts() const { return mBoatParts; }
    const std::vector<class Crystal*>& GetCrystals() const { return mCrystals; }
    const std::vector<class NPC*>& GetNPCs() const { return mNPCs; }
    const std::vector<class Platform*>& GetPlatforms() const { return mPlatforms; }
    class Key* GetKey() const { return mKey; }
    class Star* GetStar() const { return mStar; }
    PlanetShape GetPlanetShape() const { return mPlanetShape; }
    const std::unordered_map<std::string, std::vector<struct LoadedMesh>*>& GetEnemyMeshesByPath() const { return mEnemyMeshesByPath; }
    std::vector<struct LoadedMesh>* GetMeshes() const { return mMeshes; }

private:
    class Stage* mCurrentStage;
    int mStageNum;
    float mRadius;
    glm::vec4 mColor;
    std::vector<class Enemy*> mEnemies;
    std::vector<class Boat*> mBoats;
    std::vector<class BoatParts*> mBoatParts;
    std::vector<class Crystal*> mCrystals;
    std::vector<class NPC*> mNPCs;
    std::vector<class Platform*> mPlatforms;
    class Key* mKey;
    class Star* mStar;
    bool mIsAllEnemiesDead;
    bool mIsAllBoatPartsCollected;

    KeySpawnCondition mKeySpawnCondition;
    PlanetShape mPlanetShape;

    std::unordered_map<std::string, std::vector<struct LoadedMesh>*> mEnemyMeshesByPath;
    std::vector<struct LoadedMesh>* mMeshes;
};