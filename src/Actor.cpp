#include "Actor.h"
#include "Component.h"
#include "Game.h"

Actor::Actor(Game* game)
: mGame(game)
{
    
}

Actor::~Actor()
{
    
}

void Actor::Update(float deltaTime)
{
    UpdateActor(deltaTime);
}

void Actor::UpdateActor(float deltaTime)
{
    
}

void Actor::AddComponent(Component* component)
{
    int myOrder = component->GetUpdateOrder();
    auto iter = mComponents.begin();
    for(;iter != mComponents.end();iter++) {
        if(myOrder < (*iter)->GetUpdateOrder()) {
            break;
        }
    }
    mComponents.insert(iter, component);
}

void Actor::RemoveComponent(Component* component)
{
    auto iter = std::find(mComponents.begin(), mComponents.end(), component);
    if (iter != mComponents.end()) {
        mComponents.erase(iter);
    }
}