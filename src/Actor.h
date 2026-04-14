#ifndef ACTOR_H
#define ACTOR_H

#include <vector>

class Actor {
public:
    Actor(class Game* game);
    ~Actor();
    virtual void Initialize();
    void Update(float deltaTime);
    virtual void UpdateActor(float deltaTime);
    void ProcessInput();
    virtual void ProcessActor();
    void AddComponent(class Component* component);
    void RemoveComponent(class Component* component);

    Game* GetGame() const { return mGame; }
private:
    std::vector<class Component*> mComponents;
    class Game* mGame;
};
#endif