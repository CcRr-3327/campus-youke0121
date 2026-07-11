#pragma once

#include <string>

/**
 * Character — 玩家角色类
 *
 * 封装角色所有核心属性（生命、经验、金币、攻防），
 * 提供创建、升级、受伤、存档序列化等完整接口。
 */
class Character {
public:
    // ---- 构造 / 析构 ----
    Character();
    Character(const std::string& name, int level, int hp, int maxHp,
              int exp, int gold, int attack, int defense);
    ~Character();

    // ---- 核心接口 ----
    void create();                              // 从用户输入创建角色
    void showInfo() const;                      // 打印角色面板
    void levelUp();                             // 升级：提升属性
    void takeDamage(int amount);                // 受到伤害（受 defense 减免）
    void gainExp(int amount);                   // 获得经验，自动升级
    void gainGold(int amount);                  // 获得金币
    bool isAlive() const;                       // 是否存活

    // ---- 存档序列化 ----
    std::string toSaveString() const;           // 序列化为一行存档字符串
    void fromSaveString(const std::string& data); // 从存档字符串恢复

    // ---- Getter / Setter ----
    const std::string& getName() const;
    int getLevel() const;
    int getHp() const;
    int getMaxHp() const;
    int getExp() const;
    int getGold() const;
    int getAttack() const;
    int getDefense() const;

    void setName(const std::string& name);
    void setLevel(int level);
    void setHp(int hp);
    void setMaxHp(int maxHp);
    void setExp(int exp);
    void setGold(int gold);
    void setAttack(int attack);
    void setDefense(int defense);

    // 战斗中使用
    int calcDamage() const;                     // 计算本次攻击伤害值

private:
    std::string m_name;
    int m_level;
    int m_hp;
    int m_maxHp;
    int m_exp;
    int m_gold;
    int m_attack;
    int m_defense;

    // 升级所需经验公式（内部辅助）
    int expToNextLevel() const;
    void applyLevelUp();
};
