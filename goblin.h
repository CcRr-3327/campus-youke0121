#pragma once

#include "Enemy.h"

/**
 * Goblin — 哥布林（继承 Enemy）
 *
 * 中等敌人，攻防均衡，会使用武器偷袭造成额外暴击。
 */
class Goblin : public Enemy {
public:
    Goblin();
    Goblin(const std::string& name, int level);

    // ---- 重写 Enemy 接口 ----
    int attack(Character& character) override;
    std::string getDescription() const override;

private:
    int m_level;
    int m_critChance;  // 暴击概率（百分比）
};
