#include "boss.h"
#include "character.h"
#include <cstdlib>
#include <iostream>

Boss::Boss()
    : Enemy("校园Boss-教导主任", 150, 150, 15, 8, 200, 100),
      m_title("教导主任"),
      m_phase(1),
      m_skillIndex(0) {
    m_skillNames.push_back("怒吼训斥");
    m_skillNames.push_back("试卷风暴");
    m_skillNames.push_back("请家长");
}

Boss::Boss(const std::string& name, int level, const std::string& title)
    : Enemy(name,
            100 + level * 50,   // hp
            100 + level * 50,   // maxHp
            10 + level * 5,     // attack
            5 + level * 3,      // defense
            100 + level * 100,  // expReward
            50 + level * 50),   // goldReward
      m_title(title),
      m_phase(1),
      m_skillIndex(0) {
    m_skillNames.push_back("怒吼训斥");
    m_skillNames.push_back("试卷风暴");
    m_skillNames.push_back("请家长");
}

int Boss::attack(Character& character) {
    int damage = 0;

    // 血量低于50%进入狂暴阶段
    if (!isEnraged() && m_hp < m_maxHp / 2) {
        m_phase = 2;
        std::cout << "教导主任狂暴了！体力不支但怒气冲天！" << std::endl;
    }

    if (m_phase == 1) {
        // 第一阶段：正常攻击
        damage = useNormalAttack(character);
    } else {
        // 第二阶段：循环使用技能
        damage = useSpecialSkill(character);
    }

    return damage;
}

std::string Boss::getDescription() const {
    return "威严的教导主任，是校园里最强的存在";
}

bool Boss::isEnraged() const {
    return m_phase >= 2;
}

void Boss::setPhase(int phase) {
    m_phase = phase;
}

int Boss::getPhase() const {
    return m_phase;
}

const std::string& Boss::getTitle() const {
    return m_title;
}

int Boss::useNormalAttack(Character& character) {
    int baseDamage = m_attack + (rand() % 5);
    character.takeDamage(baseDamage);
    const char* quotes[]={
        "是不是又旷课了？！",
        "作业写完了吗？！",
        "上课在干什么呢？！",
        "给我站好！",
        "看我怎么收拾你！"
    };
    m_lastAttackMessage = std::string(quotes[rand()%5])+" 使用了普通攻击！";
    return baseDamage;
}

int Boss::useSpecialSkill(Character& character) {
    const std::string& skillName = getNextSkillName();
    int baseDamage = m_attack + (rand() % 5);

    int totalDamage = static_cast<int>(baseDamage * 1.5);

    std::cout << "教导主任使用了 [" << skillName << "]！" << std::endl;
    const char* skillQuotes[]={
        "这道题都不会？！",
        "罚抄一百遍！",
        "请家长来一趟！",
        "给我去跑操场！"
    };
    m_lastAttackMessage = std::string(skillQuotes[rand()%4])+" 使用了 [" + skillName + "]！";
    character.takeDamage(totalDamage);
    return totalDamage;
}

const std::string& Boss::getNextSkillName() {
    const std::string& name = m_skillNames[m_skillIndex];
    m_skillIndex = (m_skillIndex + 1) % static_cast<int>(m_skillNames.size());
    return name;
}
