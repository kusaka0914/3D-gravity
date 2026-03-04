#ifndef ENEMY_H
#define ENEMY_H

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

struct Planet;

/** 敵の共通データと基底クラス */
class EnemyBase {
public:
    glm::vec3 pos{0};
    int planetIndex = 0;
    float hp = 10.0f;
    bool alive = true;
    float damageTimer = 0.0f;
    std::string modelPath = "enemy.obj";
    float drawScale = 0.25f;
    float speed = 2.0f;
    float attack = 20.0f;
    float standByAttackTimer = -1.0f;
    bool isAttack = false;

    // 敵ごとに変える必要がないのでstatic
    // 実行中に絶対に変わらないのでconstexpr
    static constexpr float Sensing = 6.0f;

    EnemyBase() = default;
    virtual ~EnemyBase() = default;

    float getRadius() const { return drawScale * 3.6f; }

    /** ステージ1ボスなど、撃破時に鍵を出現させるボスか */
    virtual bool isBoss() const { return false; }
};

/** 通常敵（追跡など共通挙動） */
class NormalEnemy : public EnemyBase {
public:
    bool isBoss() const override { return false; }
};

/** ボス敵。撃破時に鍵出現などの特別処理対象。 */
class BossEnemy : public EnemyBase {
public:
    bool isBoss() const override { return true; }
};

using EnemyPtr = std::unique_ptr<EnemyBase>;

#endif
