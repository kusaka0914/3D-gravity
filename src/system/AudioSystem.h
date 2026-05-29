#pragma once

#include <SDL_mixer.h>
#include <string>
#include <unordered_map>

class Game;

class AudioSystem {
public:
    AudioSystem(Game* game);

    void Initialize();

    void Update();

    void Shutdown();

    void AdjustVolume(int volumeBGM, int volumeSE);
    void TryChangeBGM();
    void PlayBGM(const std::string& name);
    void PlaySE(const std::string& name);

private:
    void CreateBGMList();
    void CreateSEList();
    void AddBGM(const std::string& path, const std::string& name);
    void AddSE(const std::string& path, const std::string& name);

private:
    Game* mGame;

    std::unordered_map<std::string, Mix_Music*> mBGMList;
    std::unordered_map<std::string, Mix_Chunk*> mSEList;
};