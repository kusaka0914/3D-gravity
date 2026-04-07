#include <vector>

class Actor {
public:
    Actor(class Game* game);
    void AddComponent(class Component* component);
    void RemoveComponent(class Component* component);

private:
    std::vector<class Component*> mComponents;
    class Game* mGame;
};