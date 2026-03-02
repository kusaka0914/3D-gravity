#ifndef PLANET_H
#define PLANET_H

#include <glm/glm.hpp>
#include <vector>

struct Planet {
    glm::vec3 center;
    float radius;
    glm::vec3 color;
};

void buildSphereMesh(unsigned int segmentsLat, unsigned int segmentsLong, float radius,
    std::vector<float>& outVertices, std::vector<unsigned int>& outIndices);

#endif
