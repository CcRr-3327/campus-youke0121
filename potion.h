#pragma once

#include "Item.h"
#include <string>

/**
 * Potion — 药品类（继承 Item）
 *
 * 使用后对角色施加 Buff（攻击力暂时提升）/ Debuff（防御力暂时降低）。
 * 效果持续 m_turns 回合，受回合计数器管理。
 */
class Potion : public Item {
public:
    Potion();
    Potion(int id, const std::string& name, int price,
           const std::string& description,
           const std::string& effectType, int statModifier, int turns);

    // ---- 重写 Item 接口 ----
    void use(Character& character) override;
    std::string getInfo() const override;

    // ---- Getter / Setter ----
    const std::string& getEffectType() const;
    int getStatModifier() const;
    int getTurns() const;

    void setEffectType(const std::string& type);
    void setStatModifier(int mod);
    void setTurns(int turns);

private:
    std::string m_effectType;   // "buff" 或 "debuff"
    int m_statModifier;         // 攻/防修正量
    int m_turns;                // 持续回合数
};
