#include "character.h"
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <vector>

// ============================================================
// 构造 / 析构
// ============================================================

Character::Character()
    : m_name(""), m_level(1), m_hp(100), m_maxHp(100),
      m_exp(0), m_gold(50), m_attack(10), m_defense(5) {
}

Character::Character(const std::string& name, int level, int hp, int maxHp,
                     int exp, int gold, int attack, int defense)
    : m_name(name), m_level(level), m_hp(hp), m_maxHp(maxHp),
      m_exp(exp), m_gold(gold), m_attack(attack), m_defense(defense) {
}

Character::~Character() {
}

// ============================================================
// 核心接口
// ============================================================

void Character::create() {
    std::cout << "\n========== 角色创建 ==========" << std::endl;
    std::cout << "请输入角色名称: ";
    std::getline(std::cin, m_name);

    // 属性归为默认值
    m_level  = 1;
    m_hp     = 100;
    m_maxHp  = 100;
    m_exp    = 0;
    m_gold   = 50;
    m_attack = 10;
    m_defense = 5;

    std::cout << "\n角色创建成功！" << std::endl;
    showInfo();
}

void Character::showInfo() const {
    std::cout << "\n========================================" << std::endl;
    std::cout << "           角 色 面 板" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "  名称: " << m_name << std::endl;
    std::cout << "  等级: " << m_level << std::endl;
    std::cout << "  HP  : " << m_hp << " / " << m_maxHp << std::endl;
    std::cout << "  经验: " << m_exp << " / " << expToNextLevel() << std::endl;
    std::cout << "  金币: " << m_gold << std::endl;
    std::cout << "  攻击: " << m_attack << std::endl;
    std::cout << "  防御: " << m_defense << std::endl;
    std::cout << "========================================" << std::endl;
}

void Character::levelUp() {
    applyLevelUp();
}

void Character::takeDamage(int amount) {
    // 先减防御再扣血，至少扣 1 点
    int actualDamage = amount - m_defense;
    if (actualDamage < 1) {
        actualDamage = 1;
    }
    m_hp -= actualDamage;
    if (m_hp < 0) {
        m_hp = 0;
    }
}

void Character::gainExp(int amount) {
    std::cout << "  +" << amount << " 经验值！" << std::endl;
    m_exp += amount;

    // 支持连续多级升级
    while (m_exp >= expToNextLevel()) {
        m_exp -= expToNextLevel();
        levelUp();
        std::cout << "\n  ★★★ 升级！当前等级: " << m_level << " ★★★" << std::endl;
    }
}

void Character::gainGold(int amount) {
    std::cout << "  +" << amount << " 金币！" << std::endl;
    m_gold += amount;
    if (m_gold < 0) {
        m_gold = 0;
    }
}

bool Character::isAlive() const {
    return m_hp > 0;
}

// ============================================================
// 存档序列化
// ============================================================

std::string Character::toSaveString() const {
    // 格式: name|level|hp|maxHp|exp|gold|attack|defense
    std::ostringstream oss;
    oss << m_name   << "|"
        << m_level  << "|"
        << m_hp     << "|"
        << m_maxHp  << "|"
        << m_exp    << "|"
        << m_gold   << "|"
        << m_attack << "|"
        << m_defense;
    return oss.str();
}

void Character::fromSaveString(const std::string& data) {
    // 按 | 分隔解析
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(data);

    while (std::getline(tokenStream, token, '|')) {
        tokens.push_back(token);
    }

    if (tokens.size() >= 8) {
        m_name    = tokens[0];
        m_level   = std::stoi(tokens[1]);
        m_hp      = std::stoi(tokens[2]);
        m_maxHp   = std::stoi(tokens[3]);
        m_exp     = std::stoi(tokens[4]);
        m_gold    = std::stoi(tokens[5]);
        m_attack  = std::stoi(tokens[6]);
        m_defense = std::stoi(tokens[7]);
    }
}

// ============================================================
// Getter
// ============================================================

const std::string& Character::getName() const { return m_name; }
int Character::getLevel()  const { return m_level;  }
int Character::getHp()     const { return m_hp;     }
int Character::getMaxHp()  const { return m_maxHp;  }
int Character::getExp()    const { return m_exp;    }
int Character::getGold()   const { return m_gold;   }
int Character::getAttack() const { return m_attack; }
int Character::getDefense() const { return m_defense; }

// ============================================================
// Setter
// ============================================================

void Character::setName(const std::string& name) { m_name = name; }
void Character::setLevel(int level)   { m_level = level;   }
void Character::setHp(int hp)         { m_hp = hp;         }
void Character::setMaxHp(int maxHp)   { m_maxHp = maxHp;   }
void Character::setExp(int exp)       { m_exp = exp;       }
void Character::setGold(int gold)     { m_gold = gold;     }
void Character::setAttack(int attack) { m_attack = attack; }
void Character::setDefense(int defense) { m_defense = defense; }

// ============================================================
// 战斗相关
// ============================================================

int Character::calcDamage() const {
    // attack + 0~4 的随机浮动
    return m_attack + (rand() % 5);
}

// ============================================================
// 私有辅助
// ============================================================

int Character::expToNextLevel() const {
    // 升级所需经验公式：level * 100
    return m_level * 100;
}

void Character::applyLevelUp() {
    // 等级 +1，属性成长，HP 回满
    m_level   += 1;
    m_maxHp   += 15;
    m_attack  += 3;
    m_defense += 2;
    m_hp       = m_maxHp;  // HP 回满
}
