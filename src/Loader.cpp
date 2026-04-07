#include "Planet.h"
#include "Loader.h"
#include "Game.h"
#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>
#include <iostream>

bool loadPlayerFromYaml(const char* path, const std::vector<Planet>& planets,
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

bool loadPlanetsFromYaml(Game* game, const char* path, std::vector<std::unique_ptr<Planet>>& outPlanets) {
    // 惑星配列の初期化
    outPlanets.clear();
    try {
        YAML::Node root = YAML::LoadFile(path);
        // 失敗処理
        if (!root["planets"] || !root["planets"].IsSequence()) {
            std::cerr << "planet_loader: missing or invalid 'planets' sequence" << std::endl;
            return false;
        }
        // 配列の各要素を回す
        for (const YAML::Node& node : root["planets"]) {
            // 惑星の初期化
            std::unique_ptr<Planet> p = std::make_unique<Planet>(game);
            // 惑星中心を設定
            if (node["center"]) {
                float x = node["center"][0] ? node["center"][0].as<float>() : 0.0f;
                float y = node["center"][1] ? node["center"][1].as<float>() : 0.0f;
                float z = node["center"][2] ? node["center"][2].as<float>() : 0.0f;
                p->SetCenter(glm::vec3(x,y,z));
            } else {
                p->SetCenter(glm::vec3(0.0f));
            }
            // 惑星半径を設定
            float radius = node["radius"] ? node["radius"].as<float>() : 8.0f;
            p->SetRadius(radius);
            // 惑星色を設定
            if (node["color"]) {
                float r = node["color"][0] ? node["color"][0].as<float>() : 1.0f;
                float g = node["color"][1] ? node["color"][1].as<float>() : 1.0f;
                float b = node["color"][2] ? node["color"][2].as<float>() : 1.0f;
                p->SetColor(glm::vec3(r,g,b));
            } else {
                p->SetColor(glm::vec3(1.0f));
            }
            // モデルファイル名を設定
            if (node["model"]){
                std::string modelPath = node["model"].as<std::string>();
                p->SetModelPath(modelPath);
            }
            // 惑星配列に追加
            outPlanets.emplace_back(std::move(p));
        }
        return true;
    } catch (const YAML::Exception& ex) {
        std::cerr << "planet_loader: " << ex.what() << std::endl;
        return false;
    }
}