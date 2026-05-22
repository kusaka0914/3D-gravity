#pragma once

#include <unordered_map>
#include <string>
#include <SDL_mixer.h>

class Game;

class AudioSystem {
public:
    AudioSystem(Game* game);

    void Initialize();

    void Update();

    void Shutdown();

    void AdjustVolume(int volumeBGM, int volumeSE);
    void TryChangeBGM();
    void PlayBGM(std::string name);
    void PlaySE(std::string name);
private:
    void CreateBGMList();
    void CreateSEList();
    void AddBGM(std::string path, std::string name);
    void AddSE(std::string path, std::string name);

private:
    Game* mGame;
    
    std::unordered_map<std::string, Mix_Music*> mBGMList;
    std::unordered_map<std::string, Mix_Chunk*> mSEList;
};