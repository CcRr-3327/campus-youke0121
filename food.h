#pragma once

#include "Item.h"

/**
 * Food — 食物类（继承 Item）
 *
 * 使用后直接恢复角色 HP，不可超过 maxHp。
 */
class Food : public Item {
public:
    Food();
    Food(int id, const std::string& name, int price,
         const std::string& description, int healAmount);

    // ---- 重写 Item 接口 ----
    void use(Character& character) override;
    std::string getInfo() const override;

    // ---- Getter / Setter ----
    int getHealAmount() const;
    void setHealAmount(int amount);

private:
    int m_healAmount;  // 回复的 HP 量
};
