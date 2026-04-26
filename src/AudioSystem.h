#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <SDL_mixer.h>

class AudioSystem {
public:
    AudioSystem(class Game* game);
    void Initialize();
    void Update();
    void Shutdown();
    void PlayBGM(std::string name);
    void PlaySE(std::string name);

    Game* GetGame() const { return mGame; }
private:
    void AddBGM(std::string path, std::string name);
    void AddSE(std::string path, std::string name);

private:
    class Game* mGame;
    
    std::unordered_map<std::string, Mix_Music*> mBGMList;
    std::unordered_map<std::string, Mix_Chunk*> mSEList;
    int mCurrentBgmPlanetIndex;
};