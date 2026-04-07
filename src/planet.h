#ifndef PLANET_H
#define PLANET_H

#include <glm/glm.hpp>
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
private:
    glm::vec3 mCenter;
    float mRadius;
    glm::vec3 mColor;
    std::string mModelPath;
    std::vector<std::unique_ptr<class Enemy>> mEnemies;
};

#endif
