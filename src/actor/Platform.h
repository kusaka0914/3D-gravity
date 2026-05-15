#pragma once

#include "actor/Actor.h"

class Platform : public Actor {
public:
    Platform(class Game* game);
    void UpdateActor(float deltaTime) override;

private:

};