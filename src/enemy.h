#ifndef ENEMY_H
#define ENEMY_H

#include <glm/glm.hpp>

struct EnemyState {
    glm::vec3 pos{0};
    int planetIndex = 0;
    float hp = 10.0f;
    bool alive = true;
    float damageTimer = 0.0f;

    // 敵ごとに変える必要がないのでstatic
    // 実行中に絶対に変わらないのでconstexpr
    static constexpr float Radius = 0.4f;
    static constexpr float Sensing = 3.0f;
    static constexpr float Speed = 2.0f;
    static constexpr float Attack = 20.0f;
    static constexpr float DrawScale = 0.25f;
};

#endif
