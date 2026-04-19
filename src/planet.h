#ifndef PLANET_H
#define PLANET_H

#include "Actor.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class Planet : public Actor {
public:
    Planet(class Game* game);
    void Initialize() override;
    void UpdateActor(float deltaTime) override;
    void AddEnemy(class Enemy* enemy) { mEnemies.emplace_back(enemy); }
    void AddBoat(class Boat* boat) { mBoats.emplace_back(boat); }
    void AddBoatParts(class BoatParts* boatParts) { mBoatParts.emplace_back(boatParts); }
    void AddEnemyMesh(std::string modelPath, std::vector<LoadedMesh> meshes) { mEnemyMeshesByPath[modelPath] = meshes; }

    void RemoveAllEnemy() { for(int i = 0; i < mEnemies.size(); i++) mEnemies.pop_back(); }

    void SetCurrentStage(class Stage* currentStage) { mCurrentStage = currentStage; }
    void buildSphereMesh(unsigned int segmentsLat, unsigned int segmentsLong, float radius,
        std::vector<float>& outVertices, std::vector<unsigned int>& outIndices);
    void SetStageNum(int stageNum) { mStageNum = stageNum; }
    void SetCenter(glm::vec3 center) { mCenter = center; }
    void SetRadius(float radius) { mRadius = radius; }
    void SetColor(glm::vec3 color) { mColor = color; }
    void SetModelPath(std::string modelPath) { mModelPath = modelPath; }
    void SetKey(class Key* key) { mKey = key; }

    class Stage* GetCurrentStage() const { return mCurrentStage; }
    int GetStageNum() const { return mStageNum; }
    const glm::vec3& GetCenter() const { return mCenter; }
    float GetRadius() const { return mRadius; }
    const glm::vec3& GetColor() const { return mColor; }
    const std::string& GetModelPath() const { return mModelPath; }
    const std::vector<class Enemy*>& GetEnemies() const { return mEnemies; }
    const std::vector<class Boat*>& GetBoats() const { return mBoats; }
    const std::vector<class BoatParts*>& GetBoatParts() const { return mBoatParts; }
    class Key* GetKey() const { return mKey; }
    class Star* GetStar() const { return mStar; }
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
    class Key* mKey;
    class Star* mStar;

    std::unordered_map<std::string, std::vector<LoadedMesh>> mEnemyMeshesByPath;
};

#endif
