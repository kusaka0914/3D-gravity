#include "AudioSystem.h"
#include "Game.h"
#include "Player.h"
#include "GameProgressState.h"

AudioSystem::AudioSystem(Game* game)
    : mGame(game)
    , mCurrentBgmPlanetIndex(-1)
{
    Initialize();
}

void AudioSystem::Initialize()
{
    // SDL_mixtureの初期化（オーディオを開く）
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) != 0)
    {
        std::cerr << "Mix_OpenAudio error: " << Mix_GetError() << std::endl;
    }
    AddBGM("../assets/audio/normalBGM.wav", "normalBGM");
    AddBGM("../assets/audio/boss.wav", "bossBGM");
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

void AudioSystem::Update()
{
    std::vector<class Player*> players = GetGame()->GetPlayers();
    bool isStageClear = GetGame()->GetGameProgressState()->GetIsStageClear();
    // 惑星2にいるときはBGMをboss.wavに切り替え（ゲームクリア後はBGMを流さない）
    if (!isStageClear && players[0]->GetCurrentPlanetNum() != mCurrentBgmPlanetIndex)
    {
        if (players[0]->GetCurrentPlanetNum() == 0) {
            PlayBGM("normalBGM");
        }
        mCurrentBgmPlanetIndex = players[0]->GetCurrentPlanetNum();
        if (mCurrentBgmPlanetIndex == 2)
        {
            PlayBGM("bossBGM");
        }
    }
}

void AudioSystem::Shutdown() {
    // 再生中の曲を止める（オーディオは開いたままだから他の曲を流せる）
    Mix_HaltMusic();
    Mix_CloseAudio();
}

void AudioSystem::PlayBGM(std::string name) {
    auto it = mBGMList.find(name);
    Mix_Music* BGM = (it != mBGMList.end()) ? it->second : nullptr;
    if (BGM)
        Mix_PlayMusic(BGM, -1);
    else std::cout << "error" << std::endl;
}

void AudioSystem::PlaySE(std::string name) {
    auto it = mSEList.find(name);
    Mix_Chunk* SE = (it != mSEList.end()) ? it->second : nullptr;
    if (SE)
        Mix_PlayChannel(-1, SE, 0);
}

void AudioSystem::AddBGM(std::string path, std::string name) {
    Mix_Music* addBGM = Mix_LoadMUS(path.c_str());
    if (addBGM)
    {
        mBGMList[name] = addBGM;
    }
    else
    {
        std::cerr << "Mix_LoadMUS (" + name + ") error: " << Mix_GetError() << std::endl;
    }
}

void AudioSystem::AddSE(std::string path, std::string name) {
    Mix_Chunk* addSE = Mix_LoadWAV(path.c_str());
    if (addSE)
    {
        mSEList[name] = addSE;
    }
    else
    {
        std::cerr << "Mix_LoadWAV (" + name + ") error: " << Mix_GetError() << std::endl;
    }
}