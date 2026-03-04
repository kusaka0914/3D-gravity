#include "player_loader.h"
#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>
#include <cmath>
#include <iostream>

bool loadPlayerInitialFromYaml(const char* path, const std::vector<Planet>& planets,
    PlayerInitialConfig& outConfig) {
    try {
        YAML::Node root = YAML::LoadFile(path);
        YAML::Node node = root["players"] && root["players"].IsSequence() && root["players"].size() > 0
            ? root["players"][0]
            : root["initial"];
        if (!node) {
            std::cerr << "player_loader: missing 'players[0]' or 'initial'" << std::endl;
            return false;
        }
        float theta = node["theta"] ? node["theta"].as<float>() : 0.0f;
        float u = node["u"] ? node["u"].as<float>() : 0.0f;
        outConfig.planetIndex = node["planet_index"] ? node["planet_index"].as<int>() : 0;
        outConfig.hp = node["hp"] ? node["hp"].as<float>() : 100.0f;
        outConfig.attack = node["attack"] ? node["attack"].as<float>() : 10.0f;
        outConfig.cameraPitch = node["camera_pitch"] ? node["camera_pitch"].as<float>() : 0.4f;

        if (outConfig.planetIndex < 0 || static_cast<size_t>(outConfig.planetIndex) >= planets.size()) {
            std::cerr << "player_loader: invalid planet_index " << outConfig.planetIndex << std::endl;
            return false;
        }
        const Planet& p = planets[outConfig.planetIndex];
        glm::vec3 dir(std::cos(theta), std::sin(theta), u);
        float len = glm::length(dir);
        if (len < 1e-6f) dir = glm::vec3(1.0f, 0.0f, 0.0f);
        else dir /= len;
        outConfig.pos = p.center + p.radius * dir;
        return true;
    } catch (const YAML::Exception& ex) {
        std::cerr << "player_loader: " << ex.what() << std::endl;
        return false;
    }
}
