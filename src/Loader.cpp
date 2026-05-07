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
#include "NPC.h"
#include "Platform.h"
#include "TalkableComponent.h"
#include <glm/glm.hpp>
#include <iostream>

Loader::Loader(Game* game)
    :mGame(game)
{
    // Initialize();
}

// void Loader::Initialize() {
//     YAML::Node root = YAML::LoadFile("../");
// }

bool Loader::LoadDataFromYaml(bool isLoadPlayer) {
    std::string path = GetGame()->GetCurrentStagePath();
    if (!LoadPlanetsFromYaml(path.c_str())) {
        std::cerr << "Planet YAML Load failed" << std::endl;
        return false;
    }
    if (!LoadEnemiesFromYaml(path.c_str()))
    {
        std::cerr << "Enemy YAML Load failed" << std::endl;
        return false;
    }
    if (!LoadBoatsFromYaml(path.c_str()))
    {
        std::cerr << "Boats YAML Load failed" << std::endl;
        return false;
    }
    if (!LoadBoatPartsFromYaml(path.c_str()))
    {
        std::cerr << "BoatParts YAML Load failed" << std::endl;
        return false;
    }
    if (!LoadKeysFromYaml(path.c_str()))
    {
        std::cerr << "Keys YAML Load failed" << std::endl;
        return false;
    }
    if (!LoadCrystalsFromYaml(path.c_str()))
    {
        std::cerr << "Crystals YAML Load failed" << std::endl;
        return false;
    }
    if (!LoadStarFromYaml(path.c_str()))
    {
        std::cerr << "Star YAML Load failed" << std::endl;
        return false;
    }
    if (!LoadNPCsFromYaml(path.c_str())) {
        std::cerr << "NPC YAML Load failed" << std::endl;
        return false;
    }
    if (!LoadPlatformsFromYaml(path.c_str())) {
        std::cerr << "Platform YAML Load failed" << std::endl;
        return false;
    }
    if (!LoadPlayersFromYaml(path.c_str())) {
        std::cerr << "Player YAML Load failed" << std::endl;
        return false;
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

            std::string modelPath = node["modelPath"] ? node["modelPath"].as<std::string>() : "player.obj";
            player->SetModelPath(modelPath);

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

            YAML::Node playerRoot = YAML::LoadFile("../assets/data/players.yaml");
            for (auto playerNode : playerRoot["players"]){
                float hp = playerNode["hp"] ? playerNode["hp"].as<float>() : 0.0f;
                player->SetHp(hp);
                player->SetMaxHp(hp);

                float attackSpeed = playerNode["attackSpeed"] ? playerNode["attackSpeed"].as<float>() : 0.0f;
                player->SetAttackSpeed(attackSpeed);

                float attack = playerNode["attack"] ? playerNode["attack"].as<float>() : 0.0f;
                player->SetAttack(attack);

                float cameraPitch = playerNode["cameraPitch"] ? playerNode["cameraPitch"].as<float>() : 0.0f;
                player->SetCameraPitch(cameraPitch);

                float moveSpeed = playerNode["moveSpeed"] ? playerNode["moveSpeed"].as<float>() : 0.0f;
                player->SetMoveSpeed(moveSpeed);

                float dodgeDuration = playerNode["dodgeDuration"] ? playerNode["dodgeDuration"].as<float>() : 0.0f;
                player->SetDodgeDuration(dodgeDuration);

                float dodgeCooldownTime = playerNode["dodgeCooldownTime"] ? playerNode["dodgeCooldownTime"].as<float>() : 0.0f;
                player->SetDodgeCooldownTime(dodgeCooldownTime);

                float dodgeDistance = playerNode["dodgeDistance"] ? playerNode["dodgeDistance"].as<float>() : 0.0f;
                player->SetDodgeDistance(dodgeDistance);

                float normalAttackRange = playerNode["normalAttackRange"] ? playerNode["normalAttackRange"].as<float>() : 0.0f;
                player->SetNormalAttackRange(normalAttackRange);

                float normalAttackAngle = playerNode["normalAttackAngle"] ? playerNode["normalAttackAngle"].as<float>() : 0.0f;
                player->SetNormalAttackAngle(normalAttackAngle);

                float normalAttack = playerNode["normalAttack"] ? playerNode["normalAttack"].as<float>() : 0.0f;
                player->SetNormalAttack(normalAttack);

                float wideAttackRange = playerNode["wideAttackRange"] ? playerNode["wideAttackRange"].as<float>() : 0.0f;
                player->SetWideAttackRange(wideAttackRange);

                float wideAttackAngle = playerNode["wideAttackAngle"] ? playerNode["wideAttackAngle"].as<float>() : 0.0f;
                player->SetWideAttackAngle(wideAttackAngle);

                float wideAttack = playerNode["wideAttack"] ? playerNode["wideAttack"].as<float>() : 0.0f;
                player->SetWideAttack(wideAttack);

                float strongAttackRange = playerNode["strongAttackRange"] ? playerNode["strongAttackRange"].as<float>() : 0.0f;
                player->SetStrongAttackRange(strongAttackRange);

                float strongAttack = playerNode["strongAttack"] ? playerNode["strongAttack"].as<float>() : 0.0f;
                player->SetStrongAttack(strongAttack);

                float strongAttackSpeed = playerNode["strongAttackSpeed"] ? playerNode["strongAttackSpeed"].as<float>() : 0.0f;
                player->SetStrongAttackSpeed(strongAttackSpeed);

                float specialAttackCooldown = playerNode["specialAttackCooldown"] ? playerNode["specialAttackCooldown"].as<float>() : 0.0f;
                player->SetSpecialAttackCooldown(specialAttackCooldown);

                float defaultInvincibleTimer = playerNode["defaultInvincibleTimer"] ? playerNode["defaultInvincibleTimer"].as<float>() : 0.0f;
                player->SetDefaultInvincibleTimer(defaultInvincibleTimer);

                float defaultDamageTimer = playerNode["defaultDamageTimer"] ? playerNode["defaultDamageTimer"].as<float>() : 0.0f;
                player->SetDefaultDamageTimer(defaultDamageTimer);

                float defaultAttackMotionTimer = playerNode["defaultAttackMotionTimer"] ? playerNode["defaultAttackMotionTimer"].as<float>() : 0.0f;
                player->SetDefaultAttackMotionTimer(defaultAttackMotionTimer);

                float attackCooldown = playerNode["attackCooldown"] ? playerNode["attackCooldown"].as<float>() : 0.0f;
                player->SetAttackCooldown(attackCooldown);

                float lastAttackCooldown = playerNode["lastAttackCooldown"] ? playerNode["lastAttackCooldown"].as<float>() : 0.0f;
                player->SetLastAttackCooldown(lastAttackCooldown);

                float defaultAttackPressTimer = playerNode["defaultAttackPressTimer"] ? playerNode["defaultAttackPressTimer"].as<float>() : 0.0f;
                player->SetDefaultAttackPressTimer(defaultAttackPressTimer);

                float chargeMoveSpeed = playerNode["chargeMoveSpeed"] ? playerNode["chargeMoveSpeed"].as<float>() : 0.0f;
                player->SetChargeMoveSpeed(chargeMoveSpeed);

                float defaultStrongAttackTimer = playerNode["defaultStrongAttackTimer"] ? playerNode["defaultStrongAttackTimer"].as<float>() : 0.0f;
                player->SetDefaultStrongAttackTimer(defaultStrongAttackTimer);
            }

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

bool Loader::LoadNPCsFromYaml(const char* path) {
    try {
        YAML::Node root = YAML::LoadFile(path);
        if (!root["NPCs"] || !root["NPCs"].IsSequence()) {
            std::cerr << "Loader: missing or invalid 'NPCs' sequence" << std::endl;
        }
        // GetGame()->RemoveAllPlayer();
        for (const YAML::Node& node : root["NPCs"]) {
            std::unique_ptr<NPC> npc = std::make_unique<NPC>(mGame);

            float facingYaw = node["facingYaw"] ? node["facingYaw"].as<float>() : 0.0f;
            npc->SetFacingYaw(facingYaw);

            float radius = node["radius"] ? node["radius"].as<float>() : 0.75f;
            npc->SetRadius(radius);

            std::string modelPath = node["modelPath"] ? node["modelPath"].as<std::string>() : "player.obj";
            npc->SetModelPath(modelPath);

            std::string name = node["name"] ? node["name"].as<std::string>() : "";
            npc->SetName(name);

            if (node["talkTexts"]) {
                std::unique_ptr<TalkableComponent> talkableComponent = std::make_unique<TalkableComponent>(npc.get(), 100);
                npc->SetTalkableComponent(talkableComponent.get());
                npc->AddComponent(std::move(talkableComponent));
                for (auto talkTextNode : node["talkTexts"]) {
                    std::string talkText = talkTextNode.as<std::string>();
                    npc->GetTalkableComponent()->AddTalkTexts(talkText);
                }
            }

            int currentPlanetNum = node["currentPlanetNum"] ? node["currentPlanetNum"].as<int>() : 0;
            npc->SetCurrentPlanetNum(currentPlanetNum);

            Planet* currentPlanet = GetGame()->GetCurrentStage()->GetPlanets()[currentPlanetNum];
            npc->SetCurrentPlanet(currentPlanet);
            glm::vec3 pos = CalculatePos(node, currentPlanet);
            npc->SetPos(pos);

            NPC* npc_ptr = npc.get();
            GetGame()->AddActor(std::move(npc));
            currentPlanet->AddNPC(npc_ptr);
        }
        return true;
    } catch (const YAML::Exception& ex) {
        std::cerr << "NPC Load error: " << ex.what() << std::endl;
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

            currentPlanetNum = node["currentPlanetNum"] ? node["currentPlanetNum"].as<int>() : 0;
            enemy->SetCurrentPlanetNum(currentPlanetNum); 

            Planet* currentPlanet = GetGame()->GetCurrentStage()->GetPlanets()[currentPlanetNum];
            enemy->SetCurrentPlanet(currentPlanet);

            glm::vec3 pos = CalculatePos(node, currentPlanet);
            enemy->SetPos(pos);

            std::string type = node["type"] ? node["type"].as<std::string>() : "";
            if (type == "boss") 
                enemy->SetIsBoss(true);
            YAML::Node enemyRoot = YAML::LoadFile("../assets/data/enemies.yaml");
            for (auto enemyNode : enemyRoot["enemies"]){
                if (enemyNode["type"].as<std::string>() == "common") {
                    float knockBackSpeed = enemyNode["knockBackSpeed"] ? enemyNode["knockBackSpeed"].as<float>() : 0.0f;
                    enemy->SetKnockBackSpeed(knockBackSpeed);

                    float defaultAttackMotionTimer = enemyNode["defaultAttackMotionTimer"] ? enemyNode["defaultAttackMotionTimer"].as<float>() : 0.0f;
                    enemy->SetDefaultAttackMotionTimer(defaultAttackMotionTimer);

                    float attackSpeed = enemyNode["attackSpeed"] ? enemyNode["attackSpeed"].as<float>() : 0.0f;
                    enemy->SetAttackSpeed(attackSpeed);

                    float defaultLaunchedTimer = enemyNode["defaultLaunchedTimer"] ? enemyNode["defaultLaunchedTimer"].as<float>() : 0.0f;
                    enemy->SetDefaultLaunchedTimer(defaultLaunchedTimer);
                    continue;
                }

                if (type != enemyNode["type"].as<std::string>())
                    continue;

                float hp = enemyNode["hp"] ? enemyNode["hp"].as<float>() : 80.0f;
                enemy->SetHp(hp);
                enemy->SetMaxHp(hp);

                float scale = enemyNode["scale"] ? enemyNode["scale"].as<float>() : 0.25f;
                enemy->SetScale(glm::vec3(scale));

                float speed = enemyNode["speed"] ? enemyNode["speed"].as<float>() : 1.0f;
                enemy->SetSpeed(speed);

                float attack = enemyNode["attack"] ? enemyNode["attack"].as<float>() : 5.0f;
                enemy->SetAttack(attack);

                float radius = enemyNode["radius"] ? enemyNode["radius"].as<float>() : 0.75f;
                enemy->SetRadius(radius);

                int breakCountMax = enemyNode["breakCountMax"] ? enemyNode["breakCountMax"].as<int>() : 1;
                enemy->SetBreakCountMax(breakCountMax);
                enemy->SetBreakCount(breakCountMax);
                
                std::string modelPath = enemyNode["modelPath"] ? enemyNode["modelPath"].as<std::string>() : "";
                enemy->SetModelPath(modelPath);

                float defaultStandByAttackTimer = enemyNode["defaultStandByAttackTimer"] ? enemyNode["defaultStandByAttackTimer"].as<float>() : 0.0f;
                enemy->SetDefaultStandByAttackTimer(defaultStandByAttackTimer);
            }

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
        GetGame()->GetCurrentStage()->RemoveAllPlanet();
        YAML::Node root = YAML::LoadFile(path);
        // 失敗処理
        if (!root["planets"] || !root["planets"].IsSequence()) {
            std::cerr << "Loader: missing or invalid 'planets' sequence" << std::endl;
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
                planet->SetPos(glm::vec3(x,y,z));
            } else {
                planet->SetPos(glm::vec3(0.0f));
            }

            if (node["scale"]) {
                float scaleX = node["scale"][0] ? node["scale"][0].as<float>() : 1.0f;
                float scaleY = node["scale"][1] ? node["scale"][1].as<float>() : 1.0f;
                float scaleZ = node["scale"][2] ? node["scale"][2].as<float>() : 1.0f;
                glm::vec3 scale = glm::vec3(scaleX, scaleY, scaleZ);
                planet->SetScale(scale);
                planet->SetRadius(scaleX);
            }

            // 惑星色を設定
            if (node["color"]) {
                float r = node["color"][0] ? node["color"][0].as<float>() : 1.0f;
                float g = node["color"][1] ? node["color"][1].as<float>() : 1.0f;
                float b = node["color"][2] ? node["color"][2].as<float>() : 1.0f;
                float a = node["color"][3] ? node["color"][3].as<float>() : 1.0f;
                planet->SetColor(glm::vec4(r, g, b, a));
            } else {
                planet->SetColor(glm::vec4(1.0f));
            }

            // モデルファイル名を設定
            std::string modelPath = node["model"] ? node["model"].as<std::string>() : "";
            planet->SetModelPath(modelPath);

            std::string shape = node["shape"] ? node["shape"].as<std::string>() : "Normal";
            planet->SetPlanetShape(shape);

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

            int startPlanetNum = node["startPlanet"] ? node["startPlanet"].as<int>() : 0;
            Planet* currentPlanet = GetGame()->GetCurrentStage()->GetPlanets()[startPlanetNum];
            boat->SetCurrentPlanet(currentPlanet);

            int destPlanetNum = node["destPlanet"] ? node["destPlanet"].as<int>() : 0;
            Planet* destPlanet = GetGame()->GetCurrentStage()->GetPlanets()[destPlanetNum];
            boat->SetDestPlanet(destPlanet);

            int destStage = node["destStage"] ? node["destStage"].as<int>() : 0;
            boat->SetDestStage(destStage);

            glm::vec3 pos = CalculatePos(node, currentPlanet);
            boat->SetPos(pos);
            boat->SetStartPos(pos);

            // Planetsの設定後ではないと初期化不可
            boat->Initialize();

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

            std::string type = node["type"] ? node["type"].as<std::string>() : "";
            YAML::Node boatPartsRoot = YAML::LoadFile("../assets/data/boatParts.yaml");
            for (auto boatPartsNode : boatPartsRoot["boatParts"]){
                if (type != boatPartsNode["type"].as<std::string>())
                    continue;
                std::string modelPath = boatPartsNode["modelPath"] ? boatPartsNode["modelPath"].as<std::string>() : "";
                boatParts->SetModelPath(modelPath);
            }

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

            std::string type = node["type"] ? node["type"].as<std::string>() : "";
            YAML::Node crystalRoot = YAML::LoadFile("../assets/data/crystals.yaml");
            for (auto crystalNode : crystalRoot["crystals"]){
                if (type != crystalNode["type"].as<std::string>())
                    continue;

                float hp = crystalNode["hp"] ? crystalNode["hp"].as<float>() : 80.0f;
                crystal->GetDestructibleComponent()->SetDestroyHp(hp);

                float scale = crystalNode["scale"] ? crystalNode["scale"].as<float>() : 0.25f;
                crystal->SetScale(glm::vec3(scale));

                float radius = crystalNode["radius"] ? crystalNode["radius"].as<float>() : 1.0f;
                crystal->SetRadius(radius);
            }

            glm::vec3 pos = CalculatePos(node, currentPlanet);
            crystal->SetPos(pos);

            Crystal* crystal_ptr = crystal.get();
            GetGame()->AddActor(std::move(crystal));
            currentPlanet->AddCrystal(crystal_ptr);
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

bool Loader::LoadPlatformsFromYaml(const char* path)
{
    try {
        int currentPlanetNum = 0;
        YAML::Node root = YAML::LoadFile(path);
        if (!root["platforms"] || !root["platforms"].IsSequence()) {
            std::cerr << "Loader: missing or invalid 'platforms' sequence" << std::endl;
        }
        for (const YAML::Node& node : root["platforms"]) {
            currentPlanetNum = node["currentPlanetNum"] ? node["currentPlanetNum"].as<int>() : 0;
            Planet* currentPlanet = GetGame()->GetCurrentStage()->GetPlanets()[currentPlanetNum];
            currentPlanet->RemoveAllPlatforms();
        }

        for (auto node : root["platforms"]){
            std::unique_ptr<Platform> platform = std::make_unique<Platform>(GetGame());

            int currentPlanetNum = node["currentPlanetNum"] ? node["currentPlanetNum"].as<int>() : 0;
            Planet* currentPlanet = GetGame()->GetCurrentStage()->GetPlanets()[currentPlanetNum];
            platform->SetCurrentPlanet(currentPlanet);

            std::string type = node["type"] ? node["type"].as<std::string>() : "";
            YAML::Node platformRoot = YAML::LoadFile("../assets/data/platforms.yaml");
            for (auto platformNode : platformRoot["platforms"]){
                if (type != platformNode["type"].as<std::string>())
                    continue;

                std::string modelPath = platformNode["modelPath"] ? platformNode["modelPath"].as<std::string>() : "";
                platform->SetModelPath(modelPath);

                if (platformNode["scale"]) {
                    float scaleX = platformNode["scale"][0].as<float>();
                    float scaleY = platformNode["scale"][1].as<float>();
                    float scaleZ = platformNode["scale"][2].as<float>();
                    glm::vec3 scale = glm::vec3(scaleX, scaleY, scaleZ);
                    platform->SetScale(scale);
                }
            }

            glm::vec3 pos = CalculatePos(node, currentPlanet);
            platform->SetPos(pos);

            Platform* platformPtr = platform.get();
            GetGame()->AddActor(std::move(platform));
            currentPlanet->AddPlatform(platformPtr);
        }
        return true;
    } catch (const YAML::Exception& ex) {
        std::cerr << "Crystals Load error: " << ex.what() << std::endl;
        return false;
    }
}

glm::vec3 Loader::CalculatePos(YAML::Node node, Planet* currentPlanet) 
{   
    if (node["pos"]) {
        float posX = node["pos"][0].as<float>();
        float posY = node["pos"][1].as<float>();
        float posZ = node["pos"][2].as<float>();
        return glm::vec3(posX, posY, posZ);
    }
    float theta = node["theta"] ? node["theta"].as<float>() : 0.0f;
    float u = node["u"] ? node["u"].as<float>() : 0.0f;
    float height = node["height"] ? node["height"].as<float>() : 0.0f;
    glm::vec3 dir(std::cos(theta), std::sin(theta), u);
    float len = glm::length(dir);
    if (len < 1e-6f) dir = glm::vec3(1.0f, 0.0f, 0.0f);
    else dir /= len;

    glm::vec3 pos = currentPlanet->GetPos() + (currentPlanet->GetRadius() + height) * dir;
    return pos;
}
