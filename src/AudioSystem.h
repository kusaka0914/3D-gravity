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
    void PlayBGM(const char* name);
    void PlaySE(const char* name);

    Game* GetGame() const { return mGame; }
private:
    class Game* mGame;
    
    std::unordered_map<const char*, Mix_Music*> mBGMList;
    std::unordered_map<const char*, Mix_Chunk*> mSEList;
    int mCurrentBgmPlanetIndex;
};