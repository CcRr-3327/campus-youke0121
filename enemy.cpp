#include "enemy.h"

// ---- 构造函数 ----
Enemy::Enemy()
    : m_name(""), m_hp(0), m_maxHp(0),
      m_attack(0), m_defense(0),
      m_expReward(0), m_goldReward(0) {
}

Enemy::Enemy(const std::string& name, int hp, int maxHp,
             int attack, int defense, int expReward, int goldReward)
    : m_name(name), m_hp(hp), m_maxHp(maxHp),
      m_attack(attack), m_defense(defense),
      m_expReward(expReward), m_goldReward(goldReward) {
}

Enemy::~Enemy() {
}

// ---- 伤害与存活 ----
void Enemy::takeDamage(int amount) {
    // 先减防御再扣血，至少扣1点
    int actualDamage = amount - m_defense;
    if (actualDamage < 1) {
        actualDamage = 1;
    }
    m_hp -= actualDamage;
    if (m_hp < 0) {
        m_hp = 0;
    }
}

bool Enemy::isAlive() const {
    return m_hp > 0;
}

// ---- Getter ----
const std::string& Enemy::getName() const   { return m_name; }
int Enemy::getHp() const                    { return m_hp; }
int Enemy::getMaxHp() const                 { return m_maxHp; }
int Enemy::getAttack() const                { return m_attack; }
int Enemy::getDefense() const               { return m_defense; }
int Enemy::getExpReward() const             { return m_expReward; }
int Enemy::getGoldReward() const            { return m_goldReward; }

// ---- Setter ----
void Enemy::setName(const std::string& name) { m_name = name; }
void Enemy::setHp(int hp)                    { m_hp = hp; }
void Enemy::setMaxHp(int maxHp)              { m_maxHp = maxHp; }
void Enemy::setAttack(int attack)            { m_attack = attack; }
void Enemy::setDefense(int defense)          { m_defense = defense; }
void Enemy::setExpReward(int exp)            { m_expReward = exp; }
void Enemy::setGoldReward(int gold)          { m_goldReward = gold; }
void Enemy::setLastAttackMessage(const std::string& msg) { m_lastAttackMessage = msg; }
