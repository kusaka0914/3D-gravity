#include "enemy_loader.h"
#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>
#include <cmath>
#include <iostream>

bool loadEnemiesFromYaml(const char* path, const std::vector<Planet>& planets,
    std::vector<EnemyPtr>& outEnemies) {
    outEnemies.clear();
    try {
        YAML::Node root = YAML::LoadFile(path);
        if (!root["enemies"] || !root["enemies"].IsSequence()) {
            std::cerr << "enemy_loader: missing or invalid 'enemies' sequence" << std::endl;
            return false;
        }
        for (const YAML::Node& node : root["enemies"]) {
            bool isBoss = node["type"] && node["type"].as<std::string>() == "boss";
            EnemyBase* e = isBoss ? static_cast<EnemyBase*>(new BossEnemy()) : static_cast<EnemyBase*>(new NormalEnemy());

            float theta = node["theta"] ? node["theta"].as<float>() : 0.0f;
            float u = node["u"] ? node["u"].as<float>() : 0.0f;
            int planetIndex = node["planet_index"] ? node["planet_index"].as<int>() : 0;
            float hp = node["hp"] ? node["hp"].as<float>() : 10.0f;
            e->hp = hp;
            e->planetIndex = planetIndex;
            if (node["model"])
                e->modelPath = node["model"].as<std::string>();
            e->drawScale = node["scale"] ? node["scale"].as<float>() : 0.25f;
            e->speed = node["speed"] ? node["speed"].as<float>() : 2.0f;
            e->attack = node["attack"] ? node["attack"].as<float>() : 20.0f;

            if (planetIndex < 0 || static_cast<size_t>(planetIndex) >= planets.size()) {
                std::cerr << "enemy_loader: invalid planet_index " << planetIndex << std::endl;
                delete e;
                continue;
            }
            const Planet& p = planets[planetIndex];
            glm::vec3 dir(std::cos(theta), std::sin(theta), u);
            float len = glm::length(dir);
            if (len < 1e-6f) dir = glm::vec3(1.0f, 0.0f, 0.0f);
            else dir /= len;
            e->pos = p.center + p.radius * dir;

            outEnemies.push_back(EnemyPtr(e));
        }
        return true;
    } catch (const YAML::Exception& ex) {
        std::cerr << "enemy_loader: " << ex.what() << std::endl;
        return false;
    }
}
