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

void Actor::Initialize()
{
    
}

void Actor::ProcessInput()
{
    ProcessActor();
}

void Actor::ProcessActor()
{
    
}

void Actor::Update(float deltaTime)
{
    UpdateActor(deltaTime);
    for (auto& component : mComponents) {
        Component* comp = component.get();
        comp->Update(deltaTime);
    }
}

void Actor::UpdateActor(float deltaTime)
{
    
}

void Actor::AddComponent(std::unique_ptr<Component> component)
{
    int myOrder = component->GetUpdateOrder();
    auto iter = mComponents.begin();
    for(;iter != mComponents.end();iter++) {
        if(myOrder < (*iter)->GetUpdateOrder()) {
            break;
        }
    }
    mComponents.insert(iter, std::move(component));
}

void Actor::RemoveComponent(std::unique_ptr<Component> component)
{
    auto iter = std::find(mComponents.begin(), mComponents.end(), component);
    if (iter != mComponents.end()) {
        mComponents.erase(iter);
    }
}