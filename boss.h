#pragma once

#include "Enemy.h"
#include <vector>
#include <string>

/**
 * Boss — Boss 敌人（继承 Enemy）
 *
 * 高血量、高攻防，拥有多个阶段和特殊技能列表。
 * 血量低于 50% 后进入第二阶段，攻击模式改变。
 */
class Boss : public Enemy {
public:
    Boss();
    Boss(const std::string& name, int level, const std::string& title);

    // ---- 重写 Enemy 接口 ----
    int attack(Character& character) override;
    std::string getDescription() const override;

    // ---- Boss 特有接口 ----
    bool isEnraged() const;                         // 是否进入愤怒阶段
    void setPhase(int phase);                       // 设置阶段
    int getPhase() const;
    const std::string& getTitle() const;

private:
    std::string m_title;                        // Boss 头衔（如"教务处主任"）
    std::vector<std::string> m_skillNames;      // 技能名称列表
    int m_phase;                                // 当前阶段
    int m_skillIndex;                           // 下次使用的技能序号

    // 内部辅助
    int useNormalAttack(Character& character);
    int useSpecialSkill(Character& character);
    const std::string& getNextSkillName();
};
