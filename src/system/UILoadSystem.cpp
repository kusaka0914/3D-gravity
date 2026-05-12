#include <iostream>
#include "UILoadSystem.h"
#include "Game.h"

UILoadSystem::UILoadSystem(Game* game) 
    :mGame(game)
{
    Initialize();
}

void UILoadSystem::Initialize() {
    LoadUIInfo("../assets/data/UI.yaml");
}

void UILoadSystem::LoadUIInfo(const std::string& path) {
    YAML::Node root = YAML::LoadFile(path);
    
    for (auto screen : root) {
        std::string screenName = screen.first.as<std::string>();
        YAML::Node elements = screen.second;

        for (auto node : elements) {
            std::string type = node["type"].as<std::string>();
            if (type == "texture") {
                AddTextureInfo(screenName, node);
            } else if (type == "text") {
                AddTextInfo(screenName, node);
            }
        } 
    }
}

void UILoadSystem::AddTextureInfo(std::string screenName, YAML::Node node) {
    TextureInfo info;
    info.x = node["pos"][0] ? node["pos"][0].as<float>() : 0.0f;
    info.y = node["pos"][1] ? node["pos"][1].as<float>() : 0.0f;
    info.xRatio = node["posRatio"][0] ? node["posRatio"][0].as<float>() : 0.0f;
    info.yRatio = node["posRatio"][1] ? node["posRatio"][1].as<float>() : 0.0f;
    info.width = node["scale"][0] ? node["scale"][0].as<float>() : 0.0f;
    info.height = node["scale"][1] ? node["scale"][1].as<float>() : 0.0f;
    info.widthRatio = node["scaleRatio"][0] ? node["scaleRatio"][0].as<float>() : 0.0f;
    info.heightRatio = node["scaleRatio"][1] ? node["scaleRatio"][1].as<float>() : 0.0f;
    std::string textureId = screenName + "." + node["id"].as<std::string>();
    mTextureInfo[textureId] = info;
}

void UILoadSystem::AddTextInfo(std::string screenName, YAML::Node node) {
    TextInfo info;
    info.x = node["pos"][0] ? node["pos"][0].as<float>() : 0.0f;
    info.y = node["pos"][1] ? node["pos"][1].as<float>() : 0.0f;
    info.xRatio = node["posRatio"][0] ? node["posRatio"][0].as<float>() : 0.0f;
    info.yRatio = node["posRatio"][1] ? node["posRatio"][1].as<float>() : 0.0f;
    info.scale = node["scale"][0] ? node["scale"][0].as<float>() : 0.0f;
    info.scaleRatio = node["scaleRatio"][0] ? node["scaleRatio"][0].as<float>() : 0.0f;
    if (node["text"]) {
        for (auto text : node["text"]) {
            info.texts.emplace_back(text.as<std::string>());
        }
    }
    std::string textId = screenName + "." + node["id"].as<std::string>();
    mTextInfo[textId] = info;
}