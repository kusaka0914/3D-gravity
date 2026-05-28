#include <glm/glm.hpp>

class Game;
class Actor;
class Player;

class CameraSystem {
public:
    CameraSystem(Game* game);

    void ProcessInput();

    void Update(float deltaTime);

    glm::mat4 GetPlayerView(Player* player, float cameraDistance, bool isFixed = false);
    glm::mat4 GetFocusView(Actor* focusActor);
    glm::vec3 GetCameraPos() const { return mCameraPos; }

private:
    void UpdateCamera(float deltaTime);

private:
    float mCameraYaw;
    float mCameraPitch;
    float mCameraStickY;
    float mCameraStickX;

    glm::vec3 mCameraUpVec;
    glm::vec3 mCameraTargetPos;
    glm::vec3 mCameraPos;

    Game* mGame;
};