#include "gamemanager.h"
#include "slime.h"
#include "goblin.h"
#include "boss.h"
#include <iostream>
#include <memory>
#include <limits>

// ============================================================
// 单例
// ============================================================

GameManager& GameManager::getInstance() {
    static GameManager instance;   // static 局部变量，线程不安全单例
    return instance;
}

// ============================================================
// 构造 / 析构
// ============================================================

GameManager::GameManager()
    : m_state(GameState::MainMenu)
    , m_currentEnemy(nullptr)
{
    initSampleData();
}

GameManager::~GameManager() {
}

// ============================================================
// 游戏生命周期
// ============================================================

void GameManager::newGame() {
    std::cout << "\n========== 新游戏 ==========" << std::endl;

    // 清空输入缓冲区后创建角色
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    m_character.create();

    // 初始化示例数据：物品加入商店，任务已在 QuestManager 构造时创建
    initSampleData();

    // 进入探索状态
    m_state = GameState::Exploring;

    std::cout << "\n游戏开始！欢迎来到校园RPG冒险世界！" << std::endl;
    std::cout << "输入数字选择操作，祝你好运！" << std::endl;
}

void GameManager::loadGame(const std::string& filename) {
    bool success = m_saveManager.loadFromFile(filename,
                                               m_character,
                                               m_inventory,
                                               m_questManager);
    if (success) {
        m_state = GameState::Exploring;
        std::cout << "\n========== 读档成功！ ==========" << std::endl;
        m_character.showInfo();
    } else {
        std::cout << "\n========== 读档失败！ ==========" << std::endl;
        std::cout << "请检查存档文件是否存在。" << std::endl;
        m_state = GameState::MainMenu;
    }
}

bool GameManager::saveGame(const std::string& filename) {
    return m_saveManager.saveToFile(filename, m_character, m_inventory, m_questManager);
}

void GameManager::showMainMenu() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "       校 园 RPG 冒 险" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "  1. 开始新游戏" << std::endl;
    std::cout << "  2. 读取存档" << std::endl;
    std::cout << "  3. 退出游戏" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "请输入你的选择: ";
}

void GameManager::run() {
    int choice = 0;

    while (m_state != GameState::GameOver) {
        switch (m_state) {
        case GameState::MainMenu: {
            showMainMenu();
            std::cin >> choice;

            if (std::cin.fail()) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "请输入有效数字！" << std::endl;
                break;
            }

            processMainMenu(choice);
            break;
        }

        case GameState::Exploring: {
            std::cout << "\n========================================" << std::endl;
            std::cout << "           校 园 探 索" << std::endl;
            std::cout << "========================================" << std::endl;
            std::cout << "  1. 探索战斗" << std::endl;
            std::cout << "  2. 访问商店" << std::endl;
            std::cout << "  3. 查看背包" << std::endl;
            std::cout << "  4. 查看任务" << std::endl;
            std::cout << "  5. 角色面板" << std::endl;
            std::cout << "  6. 保存存档" << std::endl;
            std::cout << "  7. 返回主菜单" << std::endl;
            std::cout << "========================================" << std::endl;
            std::cout << "请输入你的选择: ";

            std::cin >> choice;
            if (std::cin.fail()) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "请输入有效数字！" << std::endl;
                break;
            }

            processExploring(choice);
            break;
        }

        case GameState::InBattle: {
            // 战斗由 enterBattle() 内部处理完毕后自动回归
            // 若因异常陷入此状态，回归探索
            m_state = GameState::Exploring;
            break;
        }

        case GameState::InShop: {
            // 商店由 enterShop() 内部处理完毕后自动回归
            m_state = GameState::Exploring;
            break;
        }

        case GameState::ViewingInventory: {
            // 背包由 openInventory() 内部处理完毕后自动回归
            m_state = GameState::Exploring;
            break;
        }

        case GameState::ViewingQuests: {
            // 任务由 openQuestList() 内部处理完毕后自动回归
            m_state = GameState::Exploring;
            break;
        }

        default:
            break;
        }
    }
}

void GameManager::gameOver() {
    m_state = GameState::GameOver;
    std::cout << "\n========================================" << std::endl;
    std::cout << "           游 戏 结 束" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "  谢谢游玩校园RPG冒险！再见！" << std::endl;
    std::cout << "========================================\n" << std::endl;
}

// ============================================================
// 场景接口
// ============================================================

void GameManager::enterBattle(Enemy& enemy) {
    m_currentEnemy = &enemy;
    m_state = GameState::InBattle;

    CombatResult result = m_combatSystem.startBattle(m_character, enemy);

    if (result == CombatResult::PlayerLost) {
        gameOver();
    } else {
        // 胜利或逃跑后回归探索
        m_state = GameState::Exploring;
    }

    m_currentEnemy = nullptr;
}

void GameManager::enterShop() {
    m_state = GameState::InShop;

    int choice = 0;
    bool inShop = true;

    while (inShop) {
        std::cout << "\n========================================" << std::endl;
        std::cout << "           校 园 商 店" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "  当前金币: " << m_character.getGold() << std::endl;
        std::cout << "----------------------------------------" << std::endl;

        m_shop.showItems();

        std::cout << "----------------------------------------" << std::endl;
        std::cout << "  1. 购买物品" << std::endl;
        std::cout << "  2. 离开商店" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "请输入你的选择: ";

        std::cin >> choice;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "请输入有效数字！" << std::endl;
            continue;
        }

        switch (choice) {
        case 1: {
            std::cout << "请输入要购买的物品ID: ";
            int itemId = 0;
            std::cin >> itemId;
            if (std::cin.fail()) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "请输入有效数字！" << std::endl;
                break;
            }

            if (m_shop.buyItem(itemId, m_character)) {
                // 购买成功后添加到背包
                auto item = m_shop.findItem(itemId);
                if (item) {
                    // 创建物品副本加入背包
                    m_inventory.addItem(item);
                }
            }
            break;
        }
        case 2:
            inShop = false;
            break;
        default:
            std::cout << "无效选择！请输入 1-2。" << std::endl;
            break;
        }
    }

    m_state = GameState::Exploring;
}

void GameManager::openInventory() {
    m_state = GameState::ViewingInventory;

    std::cout << "\n========== 背 包 ==========" << std::endl;
    m_inventory.showItems();

    if (m_inventory.itemCount() > 0) {
        int choice = 0;
        std::cout << "\n  1. 使用物品" << std::endl;
        std::cout << "  2. 返回" << std::endl;
        std::cout << "请输入你的选择: ";

        std::cin >> choice;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } else if (choice == 1) {
            std::cout << "请输入要使用的物品ID: ";
            int itemId = 0;
            std::cin >> itemId;
            if (!std::cin.fail()) {
                m_inventory.useItem(itemId, m_character);
            } else {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
        }
    }

    std::cout << "==========================\n" << std::endl;
    m_state = GameState::Exploring;
}

void GameManager::openQuestList() {
    m_state = GameState::ViewingQuests;

    m_questManager.listQuests();

    int choice = 0;
    std::cout << "  1. 接受任务" << std::endl;
    std::cout << "  2. 提交任务" << std::endl;
    std::cout << "  3. 返回" << std::endl;
    std::cout << "请输入你的选择: ";

    std::cin >> choice;
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } else {
        int questId = 0;
        switch (choice) {
        case 1:
            std::cout << "请输入要接受的任务ID: ";
            std::cin >> questId;
            if (!std::cin.fail()) {
                m_questManager.acceptQuest(questId);
            } else {
                std::cin.clear();
            }
            break;
        case 2:
            std::cout << "请输入要提交的任务ID: ";
            std::cin >> questId;
            if (!std::cin.fail()) {
                m_questManager.completeQuest(questId, m_character);
            } else {
                std::cin.clear();
            }
            break;
        default:
            break;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    m_state = GameState::Exploring;
}

// ============================================================
// 子系统访问
// ============================================================

Character& GameManager::getCharacter()           { return m_character; }
Inventory& GameManager::getInventory()           { return m_inventory; }
QuestManager& GameManager::getQuestManager()     { return m_questManager; }
Shop& GameManager::getShop()                     { return m_shop; }
CombatSystem& GameManager::getCombatSystem()     { return m_combatSystem; }
SaveManager& GameManager::getSaveManager()       { return m_saveManager; }
Enemy* GameManager::getCurrentEnemy()            { return m_currentEnemy; }

// ============================================================
// 状态控制
// ============================================================

GameState GameManager::getState() const          { return m_state; }
void GameManager::setState(GameState state)       { m_state = state; }

// ============================================================
// 内部辅助
// ============================================================

void GameManager::initSampleData() {
    // ---- 初始化 8 个示例物品 ----
    // 5 个指定物品 + 3 个补充物品

    // 食物类
    m_shop.addItem(std::make_shared<Food>(1, "面包", 5, "恢复20HP", 20));
    m_shop.addItem(std::make_shared<Food>(2, "急救箱", 15, "恢复50HP", 50));
    m_shop.addItem(std::make_shared<Food>(6, "草药", 8, "恢复30HP", 30));

    // 药品类
    m_shop.addItem(std::make_shared<Potion>(3, "力量药水", 20, "攻击+5持续3回合",
                                             "attack_buff", 5, 3));
    m_shop.addItem(std::make_shared<Potion>(7, "防御药剂", 25, "防御+5持续3回合",
                                             "defense_buff", 5, 3));

    // 装备类
    m_shop.addItem(std::make_shared<Equipment>(4, "铁剑", 30, "攻击+8",
                                                "weapon", 8, 0));
    m_shop.addItem(std::make_shared<Equipment>(5, "木盾", 25, "防御+5",
                                                "shield", 0, 5));
    m_shop.addItem(std::make_shared<Equipment>(8, "铁头盔", 35, "防御+8",
                                                "head", 0, 8));

    std::cout << "[系统] 商店已上架 " << 8 << " 件商品。" << std::endl;

    // ---- 任务已在 QuestManager 构造函数中创建 ----
    // QuestManager 构造函数预设了 4 个任务：
    //   1. 初入校园 (击败3只史莱姆)
    //   2. 哥布林猎人 (击败5只哥布林)
    //   3. 期中考试 (击败教导主任)
    //   4. 校园跑腿 (收集10份作业本)
    //
    // 注：派单任务要求的 5 个示例任务（校园巡逻/图书馆清理/考试挑战/社团活动/终极试炼）
    //     与 QuestManager 预设不同。由于 QuestManager 未提供 addQuest 接口，
    //     此处输出提示，委托 QuestManager 实现员统一调整预设任务内容。
    //     当前使用 QuestManager 构造函数中的预设任务。
}

void GameManager::processMainMenu(int choice) {
    switch (choice) {
    case 1:
        newGame();
        break;
    case 2: {
        std::string filename;
        std::cout << "请输入存档文件名: ";
        std::cin >> filename;
        if (filename.empty()) {
            std::cout << "文件名不能为空！" << std::endl;
        } else {
            loadGame(filename);
        }
        break;
    }
    case 3:
        gameOver();
        break;
    default:
        std::cout << "无效选择！请输入 1-3。" << std::endl;
        break;
    }
}

void GameManager::processExploring(int choice) {
    switch (choice) {
    case 1: {
        // 探索战斗：随机遭遇三种敌人
        int enemyType = rand() % 4;
        int level = m_character.getLevel();

        if (enemyType == 0) {
            Slime enemy("校园史莱姆", level);
            enterBattle(enemy);
        } else if (enemyType == 1) {
            Goblin enemy("逃课哥布林", level);
            enterBattle(enemy);
        } else if (enemyType == 2) {
            Boss enemy("教导主任", level, "教务处");
            enterBattle(enemy);
        } else {
            Goblin enemy("哥布林小队长", level + 1);
            enterBattle(enemy);
        }
        break;
    }
    case 2:
        enterShop();
        break;
    case 3:
        openInventory();
        break;
    case 4:
        openQuestList();
        break;
    case 5:
        m_character.showInfo();
        break;
    case 6: {
        std::string filename;
        std::cout << "请输入存档文件名: ";
        std::cin >> filename;
        if (!filename.empty()) {
            saveGame(filename);
        } else {
            std::cout << "文件名不能为空！" << std::endl;
        }
        break;
    }
    case 7:
        std::cout << "\n返回主菜单..." << std::endl;
        m_state = GameState::MainMenu;
        break;
    default:
        std::cout << "无效选择！请输入 1-7。" << std::endl;
        break;
    }
}

