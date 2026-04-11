#ifndef PLANET_H
#define PLANET_H

#include "Actor.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class Planet : public Actor {
public:
    Planet(class Game* game);
    void Initialize();
    void UpdateActor(float deltaTime) override;

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

private:
    glm::vec3 mCenter;
    float mRadius;
    glm::vec3 mColor;
    std::string mModelPath;
    std::vector<class Enemy*> mEnemies;
    std::vector<class Boat*> mBoats;
    class Key* mKey;
};

#endif
