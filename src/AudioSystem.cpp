#include "AudioSystem.h"
#include "Game.h"
#include "Player.h"

AudioSystem::AudioSystem(Game* game)
: mGame(game)
{
    
}

void AudioSystem::Update()
{
    std::vector<class Player*> players = GetGame()->GetPlayers();
    bool isStageClear = GetGame()->GetIsStageClear();
    const auto& bgmList = GetGame()->GetBGMList();
    // 惑星2にいるときはBGMをboss.wavに切り替え（ゲームクリア後はBGMを流さない）
    if (!isStageClear && players[0]->GetCurrentPlanet() != mCurrentBgmPlanetIndex)
    {
        mCurrentBgmPlanetIndex = players[0]->GetCurrentPlanet();
        auto it = bgmList.find("bossBGM");
        Mix_Music* bossBGM = (it != bgmList.end()) ? it->second : nullptr;
        it = bgmList.find("normalBGM");
        Mix_Music* normalBGM = (it != bgmList.end()) ? it->second : nullptr;
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