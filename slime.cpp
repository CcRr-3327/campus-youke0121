#include "slime.h"
#include "character.h"
#include <cstdlib>
#include <iostream>

Slime::Slime()
    : Enemy("史莱姆", 30, 30, 5, 2, 20, 10), m_level(1) {
}

Slime::Slime(const std::string& name, int level)
    : Enemy(name,
            20 + level * 10,    // hp
            20 + level * 10,    // maxHp
            3 + level * 2,      // attack
            1 + level * 1,      // defense
            10 + level * 10,    // expReward
            5 + level * 5),     // goldReward
      m_level(level) {
}

int Slime::attack(Character& character) {
    int baseDamage = m_attack + (rand() % 3);
    int totalDamage = baseDamage;

    // 10% 概率发动"分裂"攻击
    if ((rand() % 100) < 10) {
        totalDamage = baseDamage * 2;
        std::cout << "史莱姆分裂攻击！" << std::endl;
    }

    character.takeDamage(totalDamage);
    return totalDamage;
}

std::string Slime::getDescription() const {
    return "粘稠的史莱姆，看似人畜无害";
}
