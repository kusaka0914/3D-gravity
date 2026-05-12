#include "Helper.h"

float Helper::GetYawFromDirection(const glm::vec3& up, const glm::vec3& dir) {
    glm::vec3 worldLeft = glm::cross(up, glm::vec3(0, 0, 1));
    if (glm::length(worldLeft) < 0.01f){
        worldLeft = glm::normalize(glm::cross(up, glm::vec3(0, 1, 0)));
    }
    else 
        worldLeft = glm::normalize(worldLeft);
    glm::vec3 right = glm::cross(worldLeft, up);
    return std::atan2(-glm::dot(dir, worldLeft), glm::dot(dir, right));
}