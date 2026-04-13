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
    void AddEnemyMesh(std::string modelPath, std::vector<LoadedMesh> meshes) { mEnemyMeshesByPath[modelPath] = meshes; }

    void buildSphereMesh(unsigned int segmentsLat, unsigned int segmentsLong, float radius,
        std::vector<float>& outVertices, std::vector<unsigned int>& outIndices);
    void SetCenter(glm::vec3 center) { mCenter = center; }
    void SetRadius(float radius) { mRadius = radius; }
    void SetColor(glm::vec3 color) { mColor = color; }
    void SetModelPath(std::string modelPath) { mModelPath = modelPath; }

    const glm::vec3& GetCenter() const { return mCenter; }
    float GetRadius() const { return mRadius; }
    const glm::vec3& GetColor() const { return mColor; }
    const std::string& GetModelPath() const { return mModelPath; }
    const std::vector<class Enemy*>& GetEnemies() const { return mEnemies; }
    const std::vector<class Boat*>& GetBoats() const { return mBoats; }
    class Key* GetKey() const { return mKey; }
    const std::unordered_map<std::string, std::vector<LoadedMesh>>& GetEnemyMeshesByPath() const { return mEnemyMeshesByPath; }

private:
    glm::vec3 mCenter;
    float mRadius;
    glm::vec3 mColor;
    std::string mModelPath;
    std::vector<class Enemy*> mEnemies;
    std::vector<class Boat*> mBoats;
    class Key* mKey;

    std::unordered_map<std::string, std::vector<LoadedMesh>> mEnemyMeshesByPath;
};

#endif
