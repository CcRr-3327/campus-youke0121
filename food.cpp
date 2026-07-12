#include "food.h"
#include "character.h"
#include <iostream>
#include <algorithm>
#include <sstream>

// ---- 构造 ----

Food::Food()
    : Item(0, "", "Food", 0, ""), m_healAmount(0)
{
}

Food::Food(int id, const std::string& name, int price,
           const std::string& description, int healAmount)
    : Item(id, name, "Food", price, description), m_healAmount(healAmount)
{
}

// ---- 重写 Item 接口 ----

void Food::use(Character& character)
{
    int currentHp = character.getHp();
    int maxHp = character.getMaxHp();
    int newHp = std::min(currentHp + m_healAmount, maxHp);
    character.setHp(newHp);
    int actualHeal = newHp - currentHp;
    std::cout << "使用了[" << m_name << "]，恢复了" << actualHeal << "点生命！" << std::endl;
}

std::string Food::getInfo() const
{
    std::ostringstream oss;
    oss << Item::getInfo() << " | 回复HP：" << m_healAmount;
    return oss.str();
}

// ---- Getter / Setter ----

int Food::getHealAmount() const
{
    return m_healAmount;
}

void Food::setHealAmount(int amount)
{
    m_healAmount = amount;
}
