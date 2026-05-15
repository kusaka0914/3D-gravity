#include <glm/glm.hpp>

class Game;

class CameraSystem {
public:
    CameraSystem(Game* game);

    void ProcessInput();

    void Update(float deltaTime);

    glm::mat4 GetPlayerView(float cameraDistance, bool isFixed = false);
    glm::vec3 GetCameraPos() const { return mCameraPos; }

    // glm::mat4 GetView(float cameraDistance, bool isFixed);

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