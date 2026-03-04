#include "planet_loader.h"
#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>
#include <iostream>

bool loadPlanetsFromYaml(const char* path, std::vector<Planet>& outPlanets) {
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
            Planet p;
            // 惑星中心を設定
            if (node["center"]) {
                p.center.x = node["center"][0] ? node["center"][0].as<float>() : 0.0f;
                p.center.y = node["center"][1] ? node["center"][1].as<float>() : 0.0f;
                p.center.z = node["center"][2] ? node["center"][2].as<float>() : 0.0f;
            } else {
                p.center = glm::vec3(0.0f);
            }
            // 惑星半径を設定
            p.radius = node["radius"] ? node["radius"].as<float>() : 8.0f;
            // 惑星色を設定
            if (node["color"]) {
                p.color.r = node["color"][0] ? node["color"][0].as<float>() : 1.0f;
                p.color.g = node["color"][1] ? node["color"][1].as<float>() : 1.0f;
                p.color.b = node["color"][2] ? node["color"][2].as<float>() : 1.0f;
            } else {
                p.color = glm::vec3(1.0f);
            }
            // モデルファイル名を設定
            if (node["model"])
                p.modelPath = node["model"].as<std::string>();
            // 惑星配列に追加
            outPlanets.push_back(p);
        }
        return true;
    } catch (const YAML::Exception& ex) {
        std::cerr << "planet_loader: " << ex.what() << std::endl;
        return false;
    }
}
