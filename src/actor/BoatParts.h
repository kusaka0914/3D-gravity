#pragma once

#include "actor/Actor.h"

class Game;
class CollectableComponent;

class BoatParts : public Actor {
public:
    BoatParts(Game* game);
    void UpdateActor(float deltaTime) override;

private:
    void AddCollectableComponent();
    void OnObtained();

private:
    CollectableComponent* mCollectableComponent;
};