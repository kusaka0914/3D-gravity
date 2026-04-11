#include <vector>

class AudioSystem {
public:
    AudioSystem(class Game* game);
    void Update();

    Game* GetGame() const { return mGame; }
private:
    class Game* mGame;
    int mCurrentBgmPlanetIndex;
};