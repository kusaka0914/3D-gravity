#ifndef ACTOR_H
#define ACTOR_H

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

    Game* GetGame() const { return mGame; }
    virtual const glm::vec3& GetPos() const { return {0.0f, 0.0f, 0.0f}; };
private:
    std::vector<std::unique_ptr<Component>> mComponents;
    class Game* mGame;
};
#endif