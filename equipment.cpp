#include "equipment.h"
#include "character.h"
#include <iostream>
#include <sstream>
#include <map>

// ---- 静态记录：每个角色的每个槽位装备的属性加成（用于卸下旧装备时还原） ----
namespace {
    // 用字符指针作为key，记录每个角色各槽位的 (atkBonus, defBonus)
    struct SlotBonuses {
        int atkBonus;
        int defBonus;
    };
    std::map<Character*, std::map<std::string, SlotBonuses>> g_equippedBonuses;
}

// ---- 构造 ----

Equipment::Equipment()
    : Item(0, "", "Equipment", 0, ""), m_slot(""), m_atkBonus(0), m_defBonus(0)
{
}

Equipment::Equipment(int id, const std::string& name, int price,
                     const std::string& description,
                     const std::string& slot, int atkBonus, int defBonus)
    : Item(id, name, "Equipment", price, description)
    , m_slot(slot), m_atkBonus(atkBonus), m_defBonus(defBonus)
{
}

// ---- 重写 Item 接口 ----

void Equipment::use(Character& character)
{
    Character* pChar = &character;

    // 检查是否已有同槽位装备，若有则先卸下
    auto charIt = g_equippedBonuses.find(pChar);
    if (charIt != g_equippedBonuses.end()) {
        auto slotIt = charIt->second.find(m_slot);
        if (slotIt != charIt->second.end()) {
            // 卸下旧装备：还原属性
            character.setAttack(character.getAttack() - slotIt->second.atkBonus);
            character.setDefense(character.getDefense() - slotIt->second.defBonus);
            std::cout << "卸下了[" << m_slot << "]槽位的旧装备！（攻击-" 
                      << slotIt->second.atkBonus << " 防御-" << slotIt->second.defBonus << "）" << std::endl;
            charIt->second.erase(slotIt);
        }
    }

    // 装备新物品：加上新属性
    character.setAttack(character.getAttack() + m_atkBonus);
    character.setDefense(character.getDefense() + m_defBonus);

    // 记录新装备的属性加成
    SlotBonuses bonus;
    bonus.atkBonus = m_atkBonus;
    bonus.defBonus = m_defBonus;
    g_equippedBonuses[pChar][m_slot] = bonus;

    std::cout << "装备了[" << m_name << "]！攻击+" << m_atkBonus 
              << " 防御+" << m_defBonus << std::endl;
}

std::string Equipment::getInfo() const
{
    std::ostringstream oss;
    oss << Item::getInfo() << " | 槽位：" << m_slot
        << " | 攻击+" << m_atkBonus << " 防御+" << m_defBonus;
    return oss.str();
}

// ---- Getter / Setter ----

const std::string& Equipment::getSlot() const
{
    return m_slot;
}

int Equipment::getAtkBonus() const
{
    return m_atkBonus;
}

int Equipment::getDefBonus() const
{
    return m_defBonus;
}

void Equipment::setSlot(const std::string& slot)
{
    m_slot = slot;
}

void Equipment::setAtkBonus(int bonus)
{
    m_atkBonus = bonus;
}

void Equipment::setDefBonus(int bonus)
{
    m_defBonus = bonus;
}
