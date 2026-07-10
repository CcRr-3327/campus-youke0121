#pragma once

#include "Character.h"
#include "Inventory.h"
#include "QuestManager.h"
#include "Shop.h"
#include "CombatSystem.h"

/**
 * SaveManager — 存档管理器
 *
 * 负责将游戏状态（角色、背包、任务进度）序列化到文本文件，
 * 以及从文件反序列化恢复。
 */
class SaveManager {
public:
    SaveManager();
    SaveManager(const std::string& saveDir);
    ~SaveManager();

    // ---- 存档核心操作 ----
    bool saveToFile(const std::string& filename,
                    const Character& character,
                    const Inventory& inventory,
                    const QuestManager& questManager);   // 保存到文件

    bool loadFromFile(const std::string& filename,
                      Character& character,
                      Inventory& inventory,
                      QuestManager& questManager);       // 从文件加载

    // ---- 工具 ----
    bool fileExists(const std::string& filename) const;  // 检查存档存在
    void listSaveFiles() const;                           // 列出已有存档

    // ---- Getter / Setter ----
    const std::string& getSaveDir() const;
    void setSaveDir(const std::string& dir);

private:
    std::string m_saveDir;  // 存档目录路径
};
