#pragma once

#include "Quest.h"
#include <vector>

class Character;

/**
 * QuestManager — 任务管理器
 *
 * 使用 std::vector<Quest> 管理全部任务。
 * 负责接受、完成、发放奖励。
 */
class QuestManager {
public:
    QuestManager();
    ~QuestManager();

    // ---- 任务管理 ----
    void listQuests() const;                        // 列出所有任务
    void acceptQuest(int questId);                  // 按 ID 接受任务
    void completeQuest(int questId, Character& character);  // 完成任务并发放奖励
    void updateProgress(int questId, int count);    // 更新任务进度

    // ---- 查询 ----
    const Quest* findQuest(int questId) const;      // 查找任务
    Quest* mutableFindQuest(int questId);           // 可修改查找

private:
    std::vector<Quest> m_quests;                    // 任务列表
};
