#pragma once

#include <vector>
#include <glm/glm.hpp>

class Actor {
public:
    Actor(class Game* game);
    ~Actor();

    virtual void Initialize();

    void Update(float deltaTime);
    virtual void UpdateActor(float deltaTime);

    void ProcessInput();
    virtual void ProcessActor();

    void UpdateUpVec();

    void AddComponent(std::unique_ptr<class Component> component);
    void RemoveComponent(std::unique_ptr<Component> component);

    float getYawFromDirection(const glm::vec3& up, const glm::vec3& dir);

    void SetYaw(float yaw) { mYaw = yaw; }

    void SetPos(const glm::vec3& pos) { mPos = pos; }
    void SetUpVec(const glm::vec3& upVec) { mUpVec = upVec; }
    void SetScale(const glm::vec3& scale) { mScale = scale; }

    void SetModelPath(const std::string& modelPath) { mModelPath = modelPath; }

    void SetCurrentPlanet(class Planet* currentPlanet) { mCurrentPlanet = currentPlanet; }

    float GetYaw() const { return mYaw; }

    const glm::vec3& GetPos() const { return mPos; }
    const glm::vec3& GetUpVec() const { return mUpVec; }
    const glm::vec3& GetScale() const { return mScale; }

    const std::string& GetModelPath() const { return mModelPath; }

    class Game* GetGame() const { return mGame; }
    Planet* GetCurrentPlanet() const { return mCurrentPlanet; }

protected:
    float mYaw;

    Game* mGame;
    Planet* mCurrentPlanet;
    std::vector<std::unique_ptr<Component>> mComponents;

    glm::vec3 mPos;
    glm::vec3 mUpVec;
    glm::vec3 mScale;

    std::string mModelPath;
};