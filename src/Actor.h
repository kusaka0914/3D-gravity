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

    void AddComponent(std::unique_ptr<class Component> component);
    void RemoveComponent(std::unique_ptr<Component> component);

    class Game* GetGame() const { return mGame; }
private:
    Game* mGame;
    std::vector<std::unique_ptr<Component>> mComponents;
};