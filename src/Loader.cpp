#include "Planet.h"
#include "Loader.h"
#include "Game.h"
#include "Key.h"
#include "Player.h"
#include "Enemy.h"
#include "Stage.h"
#include "Boat.h"
#include "BoatParts.h"
#include "Crystal.h"
#include "Star.h"
#include <glm/glm.hpp>
#include <iostream>

Loader::Loader(Game* game)
    :mGame(game)
{

}

bool Loader::LoadDataFromYaml(bool isLoadPlayer) {
    std::string path = GetGame()->GetCurrentStagePath();
    if (!LoadPlanetsFromYaml(path.c_str())) {
        std::cerr << "Planet YAML Load failed" << std::endl;
        // return false;
    }
    if (!LoadEnemiesFromYaml(path.c_str()))
    {
        std::cerr << "Enemy YAML Load failed" << std::endl;
        // return false;
    }
    if (!LoadBoatsFromYaml(path.c_str()))
    {
        std::cerr << "Boats YAML Load failed" << std::endl;
        // return false;
    }
    if (!LoadBoatPartsFromYaml(path.c_str()))
    {
        std::cerr << "BoatParts YAML Load failed" << std::endl;
        // return false;
    }
    if (!LoadKeysFromYaml(path.c_str()))
    {
        std::cerr << "Keys YAML Load failed" << std::endl;
        // return false;
    }
    if (!LoadCrystalsFromYaml(path.c_str()))
    {
        std::cerr << "Crystals YAML Load failed" << std::endl;
        // return false;
    }
    if (!LoadStarFromYaml(path.c_str()))
    {
        std::cerr << "Star YAML Load failed" << std::endl;
        // return false;
    }
    if (!isLoadPlayer) return false;
    if (!LoadPlayersFromYaml(path.c_str())) {
        std::cerr << "Player YAML Load failed" << std::endl;
        // return false;
    }
    return true;
}

bool Loader::LoadPlayersFromYaml(const char* path) {
    try {
        YAML::Node root = YAML::LoadFile(path);
        if (!root["players"] || !root["players"].IsSequence()) {
            std::cerr << "Loader: missing or invalid 'players' sequence" << std::endl;
        }
        GetGame()->RemoveAllPlayer();
        int playerNum = 0;
        for (const YAML::Node& node : root["players"]) {
            std::unique_ptr<Player> player = std::make_unique<Player>(mGame);
            playerNum++;

            int currentPlanetNum = node["currentPlanetNum"] ? node["currentPlanetNum"].as<int>() : 0;
            player->SetCurrentPlanetNum(currentPlanetNum);
            player->SetRestartPlanetIndex(currentPlanetNum);

            float hp = node["hp"] ? node["hp"].as<float>() : 100.0f;
            player->SetHp(hp);

            float attack = node["attack"] ? node["attack"].as<float>() : 10.0f;
            player->SetAttack(attack);

            float cameraPitch = node["camera_pitch"] ? node["camera_pitch"].as<float>() : 0.4f;
            player->SetCameraPitch(cameraPitch);

            player->SetPlayerNum(playerNum);

            Planet* currentPlanet = GetGame()->GetCurrentStage()->GetPlanets()[currentPlanetNum];
            player->SetCurrentPlanet(currentPlanet);
            glm::vec3 pos = CalculatePos(node, currentPlanet);
            player->SetPos(pos);
            player->SetRestartPos(pos);

            Player* player_ptr = player.get();
            GetGame()->AddActor(std::move(player));
            GetGame()->AddPlayer(player_ptr);
        }
        return true;
    } catch (const YAML::Exception& ex) {
        std::cerr << "Player Load error: " << ex.what() << std::endl;
        return false;
    }
}

bool Loader::LoadEnemiesFromYaml(const char* path) {
    try {
        int currentPlanetNum = 0;
        YAML::Node root = YAML::LoadFile(path);
        if (!root["enemies"] || !root["enemies"].IsSequence()) {
            std::cerr << "Loader: missing or invalid 'enemies' sequence" << std::endl;
        }
        for (const YAML::Node& node : root["enemies"]) {
            currentPlanetNum = node["currentPlanetNum"] ? node["currentPlanetNum"].as<int>() : 0;
            Planet* currentPlanet = GetGame()->GetCurrentStage()->GetPlanets()[currentPlanetNum];
            currentPlanet->RemoveAllEnemy();
        }

        for (const YAML::Node& node : root["enemies"]) {
            std::unique_ptr<Enemy> enemy = std::make_unique<Enemy>(mGame);

            bool isBoss = node["isBoss"] ? node["isBoss"].as<int>() : false;
            enemy->SetIsBoss(isBoss);

            float hp = node["hp"] ? node["hp"].as<float>() : 80.0f;
            enemy->SetHp(hp);

            currentPlanetNum = node["currentPlanetNum"] ? node["currentPlanetNum"].as<int>() : 0;
            enemy->SetCurrentPlanetNum(currentPlanetNum);
            
            std::string modelPath = node["modelPath"] ? node["modelPath"].as<std::string>() : "enemy.obj";
            enemy->SetModelPath(modelPath);

            float scale = node["scale"] ? node["scale"].as<float>() : 0.25f;
            enemy->SetScale(scale);

            float speed = node["speed"] ? node["speed"].as<float>() : 1.0f;
            enemy->SetSpeed(speed);

            float attack = node["attack"] ? node["attack"].as<float>() : 5.0f;
            enemy->SetAttack(attack);

            float radius = node["radius"] ? node["radius"].as<float>() : 0.75f;
            enemy->SetRadius(radius);

            int breakCountMax = node["breakCountMax"] ? node["breakCountMax"].as<int>() : 1;
            enemy->SetBreakCountMax(breakCountMax);
            enemy->SetBreakCount(breakCountMax);


            Planet* currentPlanet = GetGame()->GetCurrentStage()->GetPlanets()[currentPlanetNum];
            enemy->SetCurrentPlanet(currentPlanet);

            glm::vec3 pos = CalculatePos(node, currentPlanet);
            enemy->SetPos(pos);

            Enemy* enemy_ptr = enemy.get();
            GetGame()->AddActor(std::move(enemy));
            currentPlanet->AddEnemy(enemy_ptr);
        }
        return true;
    } catch (const YAML::Exception& ex) {
        std::cerr << "Enemy Load error: " << ex.what() << std::endl;
        return false;
    }
}

bool Loader::LoadPlanetsFromYaml(const char* path) {
    try {
        YAML::Node root = YAML::LoadFile(path);
        // 失敗処理
        if (!root["planets"] || !root["planets"].IsSequence()) {
            std::cerr << "Loader: missing or invalid 'planets' sequence" << std::endl;
        }
        for (const YAML::Node& node : root["planets"]) {
            int stageNum = node["stageNum"] ? node["stageNum"].as<int>() : 0;
            Stage* currentStage = GetGame()->GetStages()[stageNum];
            currentStage->RemoveAllPlanet();
        }
        // 配列の各要素を回す
        for (const YAML::Node& node : root["planets"]) {
            // 惑星の初期化
            std::unique_ptr<Planet> planet = std::make_unique<Planet>(mGame);

            // 惑星中心を設定
            if (node["center"]) {
                float x = node["center"][0] ? node["center"][0].as<float>() : 0.0f;
                float y = node["center"][1] ? node["center"][1].as<float>() : 0.0f;
                float z = node["center"][2] ? node["center"][2].as<float>() : 0.0f;
                planet->SetCenter(glm::vec3(x,y,z));
            } else {
                planet->SetCenter(glm::vec3(0.0f));
            }

            // 惑星半径を設定
            float radius = node["radius"] ? node["radius"].as<float>() : 8.0f;
            planet->SetRadius(radius);

            // 惑星色を設定
            if (node["color"]) {
                float r = node["color"][0] ? node["color"][0].as<float>() : 1.0f;
                float g = node["color"][1] ? node["color"][1].as<float>() : 1.0f;
                float b = node["color"][2] ? node["color"][2].as<float>() : 1.0f;
                planet->SetColor(glm::vec3(r,g,b));
            } else {
                planet->SetColor(glm::vec3(1.0f));
            }

            // モデルファイル名を設定
            std::string modelPath = node["model"] ? node["model"].as<std::string>() : "";
            planet->SetModelPath(modelPath);

            std::string type = node["type"] ? node["type"].as<std::string>() : "Normal";
            planet->SetPlanetType(type);

            // ステージ番号を設定
            int stageNum = node["stageNum"] ? node["stageNum"].as<int>() : 0;
            planet->SetStageNum(stageNum);

            std::string keySpawnCondition = node["keySpawnCondition"] ? node["keySpawnCondition"].as<std::string>() : "";
            planet->SetKeySpawnCondition(keySpawnCondition);

            Stage* currentStage = GetGame()->GetCurrentStage();
            // 惑星配列に追加
            planet->SetCurrentStage(currentStage);

            planet->Initialize();
            Planet* planet_ptr = planet.get();
            GetGame()->AddActor(std::move(planet));
            currentStage->AddPlanet(planet_ptr);
        }
        return true;
    } catch (const YAML::Exception& ex) {
        std::cerr << "Planet Load error: " << ex.what() << std::endl;
        return false;
    }
}

bool Loader::LoadBoatsFromYaml(const char* path)
{
    try {
        int currentPlanetNum = 0;
        YAML::Node root = YAML::LoadFile(path);
        if (!root["boats"] || !root["boats"].IsSequence()) {
            std::cerr << "Loader: missing or invalid 'boats' sequence" << std::endl;
        }
        for (const YAML::Node& node : root["boats"]) {
            currentPlanetNum = node["currentPlanetNum"] ? node["currentPlanetNum"].as<int>() : 0;
            Planet* currentPlanet = GetGame()->GetCurrentStage()->GetPlanets()[currentPlanetNum];
            currentPlanet->RemoveAllBoat();
        }

        for (auto node : root["boats"]){
            std::unique_ptr<Boat> boat = std::make_unique<Boat>(GetGame());

            int startPlanet = node["startPlanet"] ? node["startPlanet"].as<int>() : 0;
            boat->SetStartPlanet(startPlanet);
            boat->SetCurrentPlanetNum(startPlanet);

            int destPlanet = node["destPlanet"] ? node["destPlanet"].as<int>() : 0;
            boat->SetDestPlanet(destPlanet);

            int destStage = node["destStage"] ? node["destStage"].as<int>() : 0;
            boat->SetDestStage(destStage);

            Planet* currentPlanet = GetGame()->GetCurrentStage()->GetPlanets()[startPlanet];
            boat->SetCurrentPlanet(currentPlanet);

            glm::vec3 pos = CalculatePos(node, currentPlanet);
            boat->SetPos(pos);
            boat->SetStartPos(pos);

            boat->SetPlanets(GetGame()->GetCurrentStage()->GetPlanets());

            Boat* boat_ptr = boat.get();
            GetGame()->AddActor(std::move(boat));
            currentPlanet->AddBoat(boat_ptr);
        }
        return true;
    } catch (const YAML::Exception& ex) {
        std::cerr << "Boats Load error: " << ex.what() << std::endl;
        return false;
    }
}

bool Loader::LoadBoatPartsFromYaml(const char* path)
{
    try {
        int currentPlanetNum = 0;
        YAML::Node root = YAML::LoadFile(path);
        if (!root["boatParts"] || !root["boatParts"].IsSequence()) {
            std::cerr << "Loader: missing or invalid 'boatParts' sequence" << std::endl;
        }
        for (const YAML::Node& node : root["boatParts"]) {
            currentPlanetNum = node["currentPlanetNum"] ? node["currentPlanetNum"].as<int>() : 0;
            Planet* currentPlanet = GetGame()->GetCurrentStage()->GetPlanets()[currentPlanetNum];
            currentPlanet->RemoveAllBoatParts();
        }

        for (auto node : root["boatParts"]){
            std::unique_ptr<BoatParts> boatParts = std::make_unique<BoatParts>(GetGame());

            int currentPlanetNum = node["currentPlanetNum"] ? node["currentPlanetNum"].as<int>() : 0;
            Planet* currentPlanet = GetGame()->GetCurrentStage()->GetPlanets()[currentPlanetNum];
            boatParts->SetCurrentPlanet(currentPlanet);

            glm::vec3 pos = CalculatePos(node, currentPlanet);
            boatParts->SetPos(pos);

            std::string modelPath = node["modelPath"] ? node["modelPath"].as<std::string>() : "";
            boatParts->SetModelPath(modelPath);

            BoatParts* boatParts_ptr = boatParts.get();
            GetGame()->AddActor(std::move(boatParts));
            currentPlanet->AddBoatParts(boatParts_ptr);
        }
        return true;
    } catch (const YAML::Exception& ex) {
        std::cerr << "Boats Load error: " << ex.what() << std::endl;
        return false;
    }
}

bool Loader::LoadKeysFromYaml(const char* path)
{
    try {
        int currentPlanetNum = 0;
        YAML::Node root = YAML::LoadFile(path);
        if (!root["keys"] || !root["keys"].IsSequence()) {
            std::cerr << "Loader: missing or invalid 'keys' sequence" << std::endl;
        }
        for (const YAML::Node& node : root["keys"]) {
            currentPlanetNum = node["currentPlanetNum"] ? node["currentPlanetNum"].as<int>() : 0;
            Planet* currentPlanet = GetGame()->GetCurrentStage()->GetPlanets()[currentPlanetNum];
            currentPlanet->RemoveKey();
        }

        for (auto node : root["keys"]){
            std::unique_ptr<Key> key = std::make_unique<Key>(GetGame());

            int currentPlanetNum = node["currentPlanetNum"] ? node["currentPlanetNum"].as<int>() : 0;
            Planet* currentPlanet = GetGame()->GetCurrentStage()->GetPlanets()[currentPlanetNum];
            key->SetCurrentPlanet(currentPlanet);

            glm::vec3 pos = CalculatePos(node, currentPlanet);
            key->SetPos(pos);

            Key* key_ptr = key.get();
            GetGame()->AddActor(std::move(key));
            currentPlanet->SetKey(key_ptr);
        }
        return true;
    } catch (const YAML::Exception& ex) {
        std::cerr << "Boats Load error: " << ex.what() << std::endl;
        return false;
    }
}

bool Loader::LoadCrystalsFromYaml(const char* path)
{
    try {
        int currentPlanetNum = 0;
        YAML::Node root = YAML::LoadFile(path);
        if (!root["crystals"] || !root["crystals"].IsSequence()) {
            std::cerr << "Loader: missing or invalid 'crystals' sequence" << std::endl;
        }
        for (const YAML::Node& node : root["crystals"]) {
            currentPlanetNum = node["currentPlanetNum"] ? node["currentPlanetNum"].as<int>() : 0;
            Planet* currentPlanet = GetGame()->GetCurrentStage()->GetPlanets()[currentPlanetNum];
            currentPlanet->RemoveAllCrystals();
        }

        for (auto node : root["crystals"]){
            std::unique_ptr<Crystal> crystal = std::make_unique<Crystal>(GetGame());

            int currentPlanetNum = node["currentPlanetNum"] ? node["currentPlanetNum"].as<int>() : 0;
            Planet* currentPlanet = GetGame()->GetCurrentStage()->GetPlanets()[currentPlanetNum];
            crystal->SetCurrentPlanet(currentPlanet);

            glm::vec3 pos = CalculatePos(node, currentPlanet);
            crystal->SetPos(pos);

            float scale = node["scale"] ? node["scale"].as<float>() : 0.0f;
            crystal->SetScale(scale);

            float radius = node["radius"] ? node["radius"].as<float>() : 0.0f;
            crystal->SetRadius(radius);

            float hp = node["hp"] ? node["hp"].as<float>() : 10.0;
            crystal->GetDestructibleComponent()->SetDestroyHp(hp);

            Crystal* crystal_ptr = crystal.get();
            GetGame()->AddActor(std::move(crystal));
            currentPlanet->AddCrystals(crystal_ptr);
        }
        return true;
    } catch (const YAML::Exception& ex) {
        std::cerr << "Crystals Load error: " << ex.what() << std::endl;
        return false;
    }
}

bool Loader::LoadStarFromYaml(const char* path) {
    try {
        int currentPlanetNum = 0;
        YAML::Node root = YAML::LoadFile(path);
        if (!root["star"] || !root["star"].IsSequence()) {
            std::cerr << "Loader: missing or invalid 'star' sequence" << std::endl;
        }
        // for (const YAML::Node& node : root["star"]) {
        //     currentPlanetNum = node["currentPlanetNum"] ? node["currentPlanetNum"].as<int>() : 0;
        //     Planet* currentPlanet = GetGame()->GetCurrentStage()->GetPlanets()[currentPlanetNum];
        //     currentPlanet->RemoveAllCrystals();
        // }

        for (auto node : root["star"]){
            std::unique_ptr<Star> star = std::make_unique<Star>(GetGame());

            int currentPlanetNum = node["currentPlanetNum"] ? node["currentPlanetNum"].as<int>() : 0;
            Planet* currentPlanet = GetGame()->GetCurrentStage()->GetPlanets()[currentPlanetNum];
            star->SetCurrentPlanet(currentPlanet);
            
            Star* starPtr = star.get();
            GetGame()->AddActor(std::move(star));
            currentPlanet->SetStar(starPtr);
        }
        return true;
    } catch (const YAML::Exception& ex) {
        std::cerr << "Star Load error: " << ex.what() << std::endl;
        return false;
    }
}

glm::vec3 Loader::CalculatePos(YAML::Node node, Planet* currentPlanet) 
{   
    if (node["pos"]) {
        float posX = node["pos"][0].as<float>();
        float posY = node["pos"][1].as<float>();
        float posZ = node["pos"][2].as<float>();
        std::cout << posY << std::endl;
        return glm::vec3(posX, posY, posZ);
    }
    float theta = node["theta"] ? node["theta"].as<float>() : 0.0f;
    float u = node["u"] ? node["u"].as<float>() : 0.0f;
    float height = node["height"] ? node["height"].as<float>() : 0.0f;
    glm::vec3 dir(std::cos(theta), std::sin(theta), u);
    float len = glm::length(dir);
    if (len < 1e-6f) dir = glm::vec3(1.0f, 0.0f, 0.0f);
    else dir /= len;

    glm::vec3 pos = currentPlanet->GetCenter() + (currentPlanet->GetRadius() + height) * dir;
    return pos;
}
