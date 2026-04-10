#ifndef PLANET_H
#define PLANET_H

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class Planet : Actor {
public:
    Planet(class Game* game);
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
    std::vector<std::unique_ptr<class Enemy>>& GetEnemies() { return mEnemies; }
    const std::vector<std::unique_ptr<class Enemy>>& GetEnemies() const { return mEnemies; }
    const std::vector<std::unique_ptr<class Boat>>& GetBoats() const { return mBoats; }

private:
    glm::vec3 mCenter;
    float mRadius;
    glm::vec3 mColor;
    std::string mModelPath;
    std::vector<std::unique_ptr<class Enemy>> mEnemies;
    std::vector<std::unique_ptr<class Boat>> mBoats;
};

#endif
