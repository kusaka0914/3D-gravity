#ifndef ACTOR_H
#define ACTOR_H

#include <vector>

class Actor {
public:
    Actor(class Game* game);
    ~Actor();
    void Update(float deltaTime);
    virtual void UpdateActor(float deltaTime);
    void AddComponent(class Component* component);
    void RemoveComponent(class Component* component);

    Game* GetGame() const { return mGame; }
private:
    std::vector<class Component*> mComponents;
    class Game* mGame;
};
#endif