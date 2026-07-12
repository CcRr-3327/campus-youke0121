#include "questmanager.h"
#include "character.h"
#include <iostream>

QuestManager::QuestManager() {
    // 初始化预设任务
    m_quests.push_back(Quest(1, "初入校园",
        "欢迎来到校园冒险！击败3只史莱姆来熟悉战斗系统。",
        50, 30, 3));

    m_quests.push_back(Quest(2, "哥布林猎人",
        "校园角落出现了哥布林，击败5只哥布林保护同学们的安全。",
        100, 50, 5));

    m_quests.push_back(Quest(3, "期中考试",
        "击败教导主任证明你的实力！这是校园里最强的挑战。",
        500, 200, 1));

    m_quests.push_back(Quest(4, "校园跑腿",
        "帮老师收集10份作业本，在打败史莱姆时有机会获得。",
        30, 15, 10));
}

QuestManager::~QuestManager() {
}

void QuestManager::listQuests() const {
    std::cout << "\n========== 任务列表 ==========" << std::endl;
    if (m_quests.empty()) {
        std::cout << "  暂无可用任务。" << std::endl;
    } else {
        for (const auto& quest : m_quests) {
            std::cout << "----------------------------------------" << std::endl;
            std::cout << "  ID: " << quest.getId()
                      << " | 名称: " << quest.getName() << std::endl;
            std::cout << "  描述: " << quest.getDescription() << std::endl;
            std::cout << "  进度: " << quest.getCurrentCount()
                      << "/" << quest.getTargetCount() << std::endl;
            std::cout << "  奖励: " << quest.getRewardExp()
                      << " 经验 | " << quest.getRewardGold() << " 金币" << std::endl;
            std::cout << "  状态: ";
            if (quest.isCompleted()) {
                std::cout << "已完成" << std::endl;
            } else if (quest.isAccepted()) {
                std::cout << "进行中" << std::endl;
            } else {
                std::cout << "未接受" << std::endl;
            }
        }
    }
    std::cout << "================================\n" << std::endl;
}

void QuestManager::acceptQuest(int questId) {
    Quest* quest = mutableFindQuest(questId);
    if (quest == nullptr) {
        std::cout << "找不到ID为 " << questId << " 的任务！" << std::endl;
        return;
    }
    if (quest->isCompleted()) {
        std::cout << "任务 [" << quest->getName() << "] 已经完成了！" << std::endl;
        return;
    }
    if (quest->isAccepted()) {
        std::cout << "任务 [" << quest->getName() << "] 已经接受过了！" << std::endl;
        return;
    }
    quest->accept();
}

void QuestManager::completeQuest(int questId, Character& character) {
    Quest* quest = mutableFindQuest(questId);
    if (quest == nullptr) {
        std::cout << "找不到ID为 " << questId << " 的任务！" << std::endl;
        return;
    }
    if (quest->isCompleted()) {
        std::cout << "任务 [" << quest->getName() << "] 已经完成了！" << std::endl;
        return;
    }
    if (!quest->isAccepted()) {
        std::cout << "请先接受任务 [" << quest->getName() << "]！" << std::endl;
        return;
    }
    if (!quest->checkComplete()) {
        std::cout << "任务 [" << quest->getName() << "] 尚未满足完成条件！"
                  << " (" << quest->getCurrentCount() << "/"
                  << quest->getTargetCount() << ")" << std::endl;
        return;
    }

    quest->complete();

    // 发放奖励
    int expReward = quest->getRewardExp();
    int goldReward = quest->getRewardGold();

    std::cout << "\n========== 任务完成！ ==========" << std::endl;
    std::cout << "  获得 " << expReward << " 点经验值！" << std::endl;
    std::cout << "  获得 " << goldReward << " 枚金币！" << std::endl;
    std::cout << "===============================\n" << std::endl;

    character.gainExp(expReward);
    character.gainGold(goldReward);
}

void QuestManager::updateProgress(int questId, int count) {
    Quest* quest = mutableFindQuest(questId);
    if (quest == nullptr) {
        return;
    }
    if (!quest->isAccepted() || quest->isCompleted()) {
        return;
    }
    quest->updateProgress(count);
}

const Quest* QuestManager::findQuest(int questId) const {
    for (const auto& quest : m_quests) {
        if (quest.getId() == questId) {
            return &quest;
        }
    }
    return nullptr;
}

Quest* QuestManager::mutableFindQuest(int questId) {
    for (auto& quest : m_quests) {
        if (quest.getId() == questId) {
            return &quest;
        }
    }
    return nullptr;
}
