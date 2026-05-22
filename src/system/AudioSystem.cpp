#include "AudioSystem.h"
#include "Game.h"
#include "actor/Player.h"
#include <iostream>

AudioSystem::AudioSystem(Game* game)
    : mGame(game)
{
    Initialize();
}

void AudioSystem::Initialize() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) != 0) {
        std::cerr << "Mix_OpenAudio error: " << Mix_GetError() << std::endl;
        return;
    }

    AdjustVolume(80, 60);

    CreateBGMList();
    CreateSEList();
}

void AudioSystem::AdjustVolume(int volumeBGM, int volumeSE) {
    Mix_VolumeMusic(volumeBGM);
    Mix_Volume(-1, volumeSE);
}

void AudioSystem::CreateBGMList() {
    AddBGM("../assets/audio/normalBGM.wav", "normalBGM");
    AddBGM("../assets/audio/boss.wav", "bossBGM");
}

void AudioSystem::CreateSEList() {
    AddSE("../assets/audio/attack.wav", "attackSE");
    AddSE("../assets/audio/attack_miss.wav", "attackMissSE");
    AddSE("../assets/audio/attack_pre.wav", "attackPreSE");
    AddSE("../assets/audio/counter.wav", "counterSE");
    AddSE("../assets/audio/clear.wav", "clearSE");
    AddSE("../assets/audio/attackAir.wav", "attackAirSE");
    AddSE("../assets/audio/Defeat.wav", "defeatSE");
    AddSE("../assets/audio/Damaged.wav", "damagedSE");
    AddSE("../assets/audio/Destroy.wav", "destroySE");
    AddSE("../assets/audio/Break.wav", "breakSE");
    AddSE("../assets/audio/Charged.wav", "chargedSE");
    AddSE("../assets/audio/ShowBoat.wav", "showBoatSE");
    AddSE("../assets/audio/ShowKey.wav", "showKeySE");
    AddSE("../assets/audio/PickUp.wav", "pickUpSE");
    AddSE("../assets/audio/Dodge.wav", "dodgeSE");
    AddSE("../assets/audio/Jump.wav", "jumpSE");
    AddSE("../assets/audio/Charging.wav", "chargingSE");
}

void AudioSystem::Update() {
    
}

void AudioSystem::TryChangeBGM() {
    std::vector<Player*> players = mGame->GetPlayers();
    int currentPlanetNum = players[0]->GetCurrentPlanetNum();
    if (currentPlanetNum == 0)
        PlayBGM("normalBGM");

    if (currentPlanetNum == 2)
        PlayBGM("bossBGM");
}

void AudioSystem::Shutdown() {
    Mix_HaltMusic();
    Mix_CloseAudio();
}

void AudioSystem::PlayBGM(std::string name) {
    auto it = mBGMList.find(name);
    Mix_Music* BGM = (it != mBGMList.end()) ? it->second : nullptr;
    if (BGM) {
        Mix_PlayMusic(BGM, -1);
        return;
    }
    
    std::cerr << "can't find BGM" << std::endl;
}

void AudioSystem::PlaySE(std::string name) {
    auto it = mSEList.find(name);
    Mix_Chunk* SE = (it != mSEList.end()) ? it->second : nullptr;
    if (SE) {
        Mix_PlayChannel(-1, SE, 0);
        return;
    }

    std::cerr << "can't find SE" << std::endl;
}

void AudioSystem::AddBGM(std::string path, std::string name) {
    Mix_Music* addBGM = Mix_LoadMUS(path.c_str());
    if (addBGM) {
        mBGMList[name] = addBGM;
        return;
    }

    std::cerr << "Mix_LoadMUS (" + name + ") error: " << Mix_GetError() << std::endl;
}

void AudioSystem::AddSE(std::string path, std::string name) {
    Mix_Chunk* addSE = Mix_LoadWAV(path.c_str());
    if (addSE) {
        mSEList[name] = addSE;
        return;
    }

    std::cerr << "Mix_LoadWAV (" + name + ") error: " << Mix_GetError() << std::endl;
}