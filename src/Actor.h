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

    void SetPos(const glm::vec3& pos) { mPos = pos; }
    void SetUpVec(const glm::vec3& upVec) { mUpVec = upVec; }

    class Game* GetGame() const { return mGame; }
    const glm::vec3& GetPos() const { return mPos; }
    const glm::vec3& GetUpVec() const { return mUpVec; }
private:
    Game* mGame;
    class Planet* mCurrentPlanet;
    std::vector<std::unique_ptr<Component>> mComponents;

    glm::vec3 mPos;
    glm::vec3 mUpVec;
};