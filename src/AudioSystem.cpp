#include "AudioSystem.h"
#include "Game.h"
#include "Player.h"

AudioSystem::AudioSystem(Game* game)
: mGame(game)
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

    // BGMをロードする
    Mix_Music* normalBGM = Mix_LoadMUS("../assets/audio/normalBGM.wav");
    if (normalBGM)
    {
        mBGMList["normalBGM"] = normalBGM;
        // BGMをループ再生
        Mix_PlayMusic(normalBGM, -1);
    }
    else
    {
        std::cerr << "Mix_LoadMUS error: " << Mix_GetError() << std::endl;
    }
    Mix_Music* bossBGM = Mix_LoadMUS("../assets/audio/boss.wav");
    if (bossBGM)
    {
        mBGMList["bossBGM"] = bossBGM;
    }
    else
    {
        std::cerr << "Mix_LoadMUS (boss BGM) error: " << Mix_GetError() << std::endl;
    }

    // SEをロードする
    Mix_Chunk* attackSE = Mix_LoadWAV("../assets/audio/attack.wav");
    if (attackSE)
    {
        mSEList["attackSE"] = attackSE;
    }
    else
    {
        std::cerr << "Mix_LoadWAV (attack SE) error: " << Mix_GetError() << std::endl;
    }
    Mix_Chunk* attackMissSE = Mix_LoadWAV("../assets/audio/attack_miss.wav");
    if (attackMissSE)
    {
        mSEList["attackMissSE"] = attackMissSE;
    }
    else
    {
        std::cerr << "Mix_LoadWAV (attack_miss SE) error: " << Mix_GetError() << std::endl;
    }
    Mix_Chunk* attackPreSE = Mix_LoadWAV("../assets/audio/attack_pre.wav");
    if (attackPreSE)
    {
        mSEList["attackPreSE"] = attackPreSE;
    }
    else
    {
        std::cerr << "Mix_LoadWAV (attack_pre SE) error: " << Mix_GetError() << std::endl;
    }
    Mix_Chunk* counterSE = Mix_LoadWAV("../assets/audio/counter.wav");
    if (counterSE)
    {
        mSEList["counterSE"] = counterSE;
    }
    else
    {
        std::cerr << "Mix_LoadWAV (counter SE) error: " << Mix_GetError() << std::endl;
    }
    Mix_Chunk* clearSE = Mix_LoadWAV("../assets/audio/clear.wav");
    if (clearSE)
    {
        mSEList["clearSE"] = clearSE;
    }
    else
    {
        std::cerr << "Mix_LoadWAV (clear SE) error: " << Mix_GetError() << std::endl;
    }
    Mix_Chunk* attackAirSE = Mix_LoadWAV("../assets/audio/attackAir.wav");
    if (attackAirSE)
    {
        mSEList["attackAirSE"] = attackAirSE;
    }
    else
    {
        std::cerr << "Mix_LoadWAV (attackAir SE) error: " << Mix_GetError() << std::endl;
    }
    Mix_Chunk* defeatSE = Mix_LoadWAV("../assets/audio/Defeat.wav");
    if (defeatSE)
    {
        mSEList["defeatSE"] = defeatSE;
    }
    else
    {
        std::cerr << "Mix_LoadWAV (defeat SE) error: " << Mix_GetError() << std::endl;
    }
    Mix_Chunk* damagedSE = Mix_LoadWAV("../assets/audio/Damaged.wav");
    if (damagedSE)
    {
        mSEList["damagedSE"] = damagedSE;
    }
    else
    {
        std::cerr << "Mix_LoadWAV (damaged SE) error: " << Mix_GetError() << std::endl;
    }
}

void AudioSystem::Update()
{
    std::vector<class Player*> players = GetGame()->GetPlayers();
    bool isStageClear = GetGame()->GetIsStageClear();
    // 惑星2にいるときはBGMをboss.wavに切り替え（ゲームクリア後はBGMを流さない）
    if (!isStageClear && players[0]->GetCurrentPlanetNum() != mCurrentBgmPlanetIndex)
    {
        mCurrentBgmPlanetIndex = players[0]->GetCurrentPlanetNum();
        auto it = mBGMList.find("bossBGM");
        Mix_Music* bossBGM = (it != mBGMList.end()) ? it->second : nullptr;
        it = mBGMList.find("normalBGM");
        Mix_Music* normalBGM = (it != mBGMList.end()) ? it->second : nullptr;
        if (mCurrentBgmPlanetIndex == 1 && bossBGM)
        {
            Mix_PlayMusic(bossBGM, -1);
        }
        else if (normalBGM)
        {
            Mix_PlayMusic(normalBGM, -1);
        }
    }
}

void AudioSystem::PlayBGM(const char* name) {
    auto it = mBGMList.find(name);
    Mix_Music* BGM = (it != mBGMList.end()) ? it->second : nullptr;
    if (BGM)
        Mix_PlayMusic(BGM, -1);
}

void AudioSystem::PlaySE(const char* name) {
    auto it = mSEList.find(name);
    Mix_Chunk* SE = (it != mSEList.end()) ? it->second : nullptr;
    if (SE)
        Mix_PlayChannel(-1, SE, 0);
}