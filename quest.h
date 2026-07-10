#pragma once

#include <string>

/**
 * Quest — 任务类
 *
 * 描述一个可接受、可完成的任务，含目标和奖励。
 * 通过 targetCount / currentCount 追踪进度。
 */
class Quest {
public:
    Quest();
    Quest(int id, const std::string& name, const std::string& description,
          int rewardExp, int rewardGold, int targetCount);

    // ---- 任务生命周期 ----
    void accept();                              // 接受任务
    void complete();                            // 完成任务
    void updateProgress(int count);             // 更新进度（currentCount += count）
    bool checkComplete() const;                 // 是否满足完成条件
    void showInfo() const;                      // 打印任务信息

    // ---- 状态查询 ----
    bool isAccepted() const;
    bool isCompleted() const;

    // ---- Getter / Setter ----
    int getId() const;
    const std::string& getName() const;
    const std::string& getDescription() const;
    int getRewardExp() const;
    int getRewardGold() const;
    int getTargetCount() const;
    int getCurrentCount() const;

    void setId(int id);
    void setName(const std::string& name);
    void setDescription(const std::string& description);
    void setRewardExp(int exp);
    void setRewardGold(int gold);
    void setTargetCount(int count);
    void setCurrentCount(int count);

private:
    int m_id;
    std::string m_name;
    std::string m_description;
    bool m_accepted;
    bool m_completed;
    int m_rewardExp;
    int m_rewardGold;
    int m_targetCount;      // 需要完成的目标数量
    int m_currentCount;     // 当前已完成数量
};
