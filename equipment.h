#pragma once

#include "Item.h"
#include <string>

/**
 * Equipment — 装备类（继承 Item）
 *
 * 使用后装备到指定槽位，获得攻击/防御加成。
 * 装备前需先卸下同类槽位已有装备。
 */
class Equipment : public Item {
public:
    Equipment();
    Equipment(int id, const std::string& name, int price,
              const std::string& description,
              const std::string& slot, int atkBonus, int defBonus);

    // ---- 重写 Item 接口 ----
    void use(Character& character) override;
    std::string getInfo() const override;

    // ---- Getter / Setter ----
    const std::string& getSlot() const;
    int getAtkBonus() const;
    int getDefBonus() const;

    void setSlot(const std::string& slot);
    void setAtkBonus(int bonus);
    void setDefBonus(int bonus);

private:
    std::string m_slot;   // "head" / "body" / "weapon" / "shield"
    int m_atkBonus;       // 攻击加成
    int m_defBonus;       // 防御加成
};
