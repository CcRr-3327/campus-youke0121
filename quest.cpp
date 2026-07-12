#include "quest.h"
#include <iostream>

// ---- 构造函数 ----
Quest::Quest()
    : m_id(0), m_name(""), m_description(""),
      m_accepted(false), m_completed(false),
      m_rewardExp(0), m_rewardGold(0),
      m_targetCount(0), m_currentCount(0) {
}

Quest::Quest(int id, const std::string& name, const std::string& description,
             int rewardExp, int rewardGold, int targetCount)
    : m_id(id), m_name(name), m_description(description),
      m_accepted(false), m_completed(false),
      m_rewardExp(rewardExp), m_rewardGold(rewardGold),
      m_targetCount(targetCount), m_currentCount(0) {
}

// ---- 任务生命周期 ----
void Quest::accept() {
    m_accepted = true;
    std::cout << "接受了任务 [" << m_name << "]！" << std::endl;
}

void Quest::complete() {
    m_completed = true;
    std::cout << "完成了任务 [" << m_name << "]！" << std::endl;
}

void Quest::updateProgress(int count) {
    m_currentCount += count;
    if (m_currentCount > m_targetCount) {
        m_currentCount = m_targetCount;
    }
}

bool Quest::checkComplete() const {
    return m_currentCount >= m_targetCount;
}

void Quest::showInfo() const {
    std::cout << "========================================" << std::endl;
    std::cout << "  任务名称: " << m_name << std::endl;
    std::cout << "  任务描述: " << m_description << std::endl;
    std::cout << "  完成条件: " << m_currentCount << "/" << m_targetCount << std::endl;
    std::cout << "  经验奖励: " << m_rewardExp << std::endl;
    std::cout << "  金币奖励: " << m_rewardGold << std::endl;

    std::cout << "  任务状态: ";
    if (m_completed) {
        std::cout << "已完成" << std::endl;
    } else if (m_accepted) {
        std::cout << "进行中" << std::endl;
    } else {
        std::cout << "未接受" << std::endl;
    }
    std::cout << "========================================" << std::endl;
}

// ---- 状态查询 ----
bool Quest::isAccepted() const {
    return m_accepted;
}

bool Quest::isCompleted() const {
    return m_completed;
}

// ---- Getter ----
int Quest::getId() const                     { return m_id; }
const std::string& Quest::getName() const    { return m_name; }
const std::string& Quest::getDescription() const { return m_description; }
int Quest::getRewardExp() const              { return m_rewardExp; }
int Quest::getRewardGold() const             { return m_rewardGold; }
int Quest::getTargetCount() const            { return m_targetCount; }
int Quest::getCurrentCount() const           { return m_currentCount; }

// ---- Setter ----
void Quest::setId(int id)                    { m_id = id; }
void Quest::setName(const std::string& name) { m_name = name; }
void Quest::setDescription(const std::string& desc) { m_description = desc; }
void Quest::setRewardExp(int exp)            { m_rewardExp = exp; }
void Quest::setRewardGold(int gold)          { m_rewardGold = gold; }
void Quest::setTargetCount(int count)        { m_targetCount = count; }
void Quest::setCurrentCount(int count)       { m_currentCount = count; }
