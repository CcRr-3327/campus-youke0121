#include "potion.h"
#include "character.h"
#include <iostream>
#include <sstream>

// ---- 构造 ----

Potion::Potion()
    : Item(0, "", "Potion", 0, ""), m_effectType("buff"), m_statModifier(0), m_turns(0)
{
}

Potion::Potion(int id, const std::string& name, int price,
               const std::string& description,
               const std::string& effectType, int statModifier, int turns)
    : Item(id, name, "Potion", price, description)
    , m_effectType(effectType), m_statModifier(statModifier), m_turns(turns)
{
}

// ---- 重写 Item 接口 ----

void Potion::use(Character& character)
{
    if (m_effectType == "attack_buff" || m_effectType == "buff") {
        int oldAttack = character.getAttack();
        character.setAttack(oldAttack + m_statModifier);
        std::cout << "[" << m_name << "]提升了攻击力" << m_statModifier
                  << "点，持续" << m_turns << "回合！（临时效果）" << std::endl;
    } else if (m_effectType == "defense_buff") {
        int oldDefense = character.getDefense();
        character.setDefense(oldDefense + m_statModifier);
        std::cout << "[" << m_name << "]提升了防御力" << m_statModifier
                  << "点，持续" << m_turns << "回合！（临时效果）" << std::endl;
    } else if (m_effectType == "debuff") {
        int oldDefense = character.getDefense();
        int newDefense = oldDefense - m_statModifier;
        if (newDefense < 0) newDefense = 0;
        character.setDefense(newDefense);
        std::cout << "[" << m_name << "]降低了防御力" << m_statModifier
                  << "点，持续" << m_turns << "回合！（临时效果）" << std::endl;
    } else {
        std::cout << "[" << m_name << "]效果类型未知，使用失败！" << std::endl;
    }
}

std::string Potion::getInfo() const
{
    std::ostringstream oss;
    oss << Item::getInfo();
    if (m_effectType == "attack_buff") {
        oss << " | 提升攻击力：" << m_statModifier << " 持续：" << m_turns << "回合";
    } else if (m_effectType == "defense_buff") {
        oss << " | 提升防御力：" << m_statModifier << " 持续：" << m_turns << "回合";
    } else if (m_effectType == "debuff") {
        oss << " | 降低防御力：" << m_statModifier << " 持续：" << m_turns << "回合";
    }
    return oss.str();
}

// ---- Getter / Setter ----

const std::string& Potion::getEffectType() const
{
    return m_effectType;
}

int Potion::getStatModifier() const
{
    return m_statModifier;
}

int Potion::getTurns() const
{
    return m_turns;
}

void Potion::setEffectType(const std::string& type)
{
    m_effectType = type;
}

void Potion::setStatModifier(int mod)
{
    m_statModifier = mod;
}

void Potion::setTurns(int turns)
{
    m_turns = turns;
}
