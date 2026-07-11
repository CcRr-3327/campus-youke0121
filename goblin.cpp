#include "goblin.h"
#include "character.h"
#include <cstdlib>
#include <iostream>

Goblin::Goblin()
    : Enemy("哥布林", 50, 50, 8, 3, 40, 20), m_level(1), m_critChance(20) {
}

Goblin::Goblin(const std::string& name, int level)
    : Enemy(name,
            30 + level * 20,    // hp
            30 + level * 20,    // maxHp
            5 + level * 3,      // attack
            2 + level * 1,      // defense
            20 + level * 20,    // expReward
            10 + level * 10),   // goldReward
      m_level(level), m_critChance(20) {
}

int Goblin::attack(Character& character) {
    int baseDamage = m_attack + (rand() % 3);
    int totalDamage = baseDamage;

    // 20% 概率暴击
    if ((rand() % 100) < m_critChance) {
        totalDamage = baseDamage * 2;
        std::cout << "哥布林暴击！" << std::endl;
    }

    character.takeDamage(totalDamage);
    return totalDamage;
}

std::string Goblin::getDescription() const {
    return "狡猾的哥布林，手持生锈的短刀";
}
