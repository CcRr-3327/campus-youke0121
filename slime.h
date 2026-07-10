#pragma once

#include "Enemy.h"

/**
 * Slime — 史莱姆（继承 Enemy）
 *
 * 最弱敌人，攻击力低，有概率分裂恢复少量HP。
 */
class Slime : public Enemy {
public:
    Slime();
    Slime(const std::string& name, int level);

    // ---- 重写 Enemy 接口 ----
    int attack(Character& character) override;
    std::string getDescription() const override;

private:
    int m_level;  // 等级影响属性缩放
};
