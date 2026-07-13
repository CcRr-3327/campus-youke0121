#pragma once

#include "Character.h"
#include "Inventory.h"
#include "QuestManager.h"
#include "Shop.h"
#include "CombatSystem.h"
#include "SaveManager.h"
#include "Enemy.h"
#include <memory>

// ---- SFML 前向声明（预留） ----
// 若编译时链接 SFML，取消以下注释并 include <SFML/Graphics.hpp>
// class sf::RenderWindow;
// class sf::Event;

/**
 * GameState — 游戏状态枚举
 */
enum class GameState {
    MainMenu,
    Exploring,
    InBattle,
    InShop,
    ViewingInventory,
    ViewingQuests,
    GameOver
};

/**
 * GameManager — 游戏管理器（单例）
 *
 * 聚合所有子系统。作为整个游戏的顶层控制器，
 * 协调各个模块之间的交互。唯一实例通过 getInstance() 获取。
 *
 * 包含 SFML 渲染相关接口（预留），未来可直接对接图形界面。
 */
class GameManager {
public:
    // ---- 单例 ----
    static GameManager& getInstance();

    // 禁止拷贝 / 移动
    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;

    // ---- 游戏生命周期 ----
    void newGame();                              // 新游戏
    void loadGame(const std::string& filename);  // 读取存档
    bool saveGame(const std::string& filename);  // 保存存档
    void showMainMenu();                         // 主菜单
    void run();                                  // 主循环（命令行版）

    // ---- 子系统访问 ----
    Character& getCharacter();
    Inventory& getInventory();
    QuestManager& getQuestManager();
    Shop& getShop();
    CombatSystem& getCombatSystem();
    SaveManager& getSaveManager();
    Enemy* getCurrentEnemy();

    // ---- 状态控制 ----
    GameState getState() const;
    void setState(GameState state);

    // ---- 场景接口 ----
    void enterBattle(Enemy& enemy);              // 进入战斗
    void enterShop();                            // 进入商店
    void openInventory();                        // 打开背包
    void openQuestList();                        // 打开任务面板
    void gameOver();                             // 游戏结束

    // ============================================================
    // ---- SFML 渲染接口（预留） ----
private:
    // ---- 构造 / 析构（私有，单例） ----
    GameManager();
    ~GameManager();

    // ---- 聚合的子系统 ----
    Character      m_character;
    Inventory      m_inventory;
    QuestManager   m_questManager;
    Shop           m_shop;
    CombatSystem   m_combatSystem;
    SaveManager    m_saveManager;

    // ---- 运行时状态 ----
    GameState      m_state;
    Enemy*         m_currentEnemy;   // 当前战斗中敌人（非拥有）

    // ---- 内部辅助 ----
    void initSampleData();           // 初始化示例数据（物品、任务等）
    void processMainMenu(int choice);
    void processExploring(int choice);
    void processBattle(int choice);
};
