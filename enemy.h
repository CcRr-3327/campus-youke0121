#pragma once

#include <string>

class Character;

/**
 * Enemy — 敌人抽象基类
 *
 * 定义敌人的公共属性和战斗行为接口。
 * 子类重写 attack() 和 getDescription() 实现多态。
 */
class Enemy {
public:
    Enemy();
    Enemy(const std::string& name, int hp, int maxHp,
          int attack, int defense, int expReward, int goldReward);
    virtual ~Enemy();

    // ---- 多态接口 ----
    virtual int attack(Character& character) = 0;  // 攻击角色，返回伤害值
    virtual std::string getDescription() const = 0; // 描述敌人

    void takeDamage(int amount);                    // 受到伤害
    bool isAlive() const;                           // 是否存活

    // ---- Getter / Setter ----
    const std::string& getName() const;
    int getHp() const;
    int getMaxHp() const;
    int getAttack() const;
    int getDefense() const;
    int getExpReward() const;
    int getGoldReward() const;

    void setName(const std::string& name);
    void setHp(int hp);
    void setMaxHp(int maxHp);
    void setAttack(int attack);
    void setDefense(int defense);
    void setExpReward(int exp);
    void setGoldReward(int gold);

protected:
    std::string m_name;
    int m_hp;
    int m_maxHp;
    int m_attack;
    int m_defense;
    int m_expReward;
    int m_goldReward;
};
