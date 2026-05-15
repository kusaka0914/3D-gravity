#pragma once

#include <vector>
#include <glm/glm.hpp>

class Game;
class Component;
class Planet;

class Actor {
public:
    Actor(Game* game);
    ~Actor();

    virtual void Initialize();

    void Update(float deltaTime);
    virtual void UpdateActor(float deltaTime);

    void ProcessInput();
    virtual void ProcessActor();

    void UpdateUpVec();
    void UpdateFallbackUpVec();
    glm::vec3 GetAverageNormal();
    virtual void OnLanded(const glm::vec3& hitPos) {};

    void AddComponent(std::unique_ptr<Component> component);
    void RemoveComponent(std::unique_ptr<Component> component);

    void SetIsActive(bool isActive) { mIsActive = isActive; }

    void SetRadius(float radius) { mRadius = radius; }

    void SetPos(const glm::vec3& pos) { mPos = pos; }
    void SetUpVec(const glm::vec3& upVec) { mUpVec = upVec; }
    void SetScale(const glm::vec3& scale) { mScale = scale; }

    void SetModelPath(const std::string& modelPath) { mModelPath = modelPath; }

    void SetCurrentPlanet(Planet* currentPlanet) { mCurrentPlanet = currentPlanet; }
    void SetMeshes(std::vector<struct LoadedMesh>* Meshes) { mMeshes = Meshes; }

    bool GetIsActive() const { return mIsActive; }

    float GetRadius() const { return mRadius; }

    const glm::vec3& GetPos() const { return mPos; }
    const glm::vec3& GetUpVec() const { return mUpVec; }
    const glm::vec3& GetScale() const { return mScale; }

    const std::string& GetModelPath() const { return mModelPath; }

    Game* GetGame() const { return mGame; }
    Planet* GetCurrentPlanet() const { return mCurrentPlanet; }
    std::vector<struct LoadedMesh>* GetMeshes() const { return mMeshes; }

protected:
    bool mIsActive;

    float mRadius;

    glm::vec3 mPos;
    glm::vec3 mUpVec;
    glm::vec3 mScale;

    std::string mModelPath;

    Game* mGame;
    Planet* mCurrentPlanet;
    std::vector<std::unique_ptr<Component>> mComponents;
    std::vector<struct LoadedMesh>* mMeshes;
};