#include "savemanager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <string>

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#define HAS_FILESYSTEM 1
#else
#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <sys/stat.h>
#define MKDIR(p) mkdir(p, 0755)
#endif
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

static std::string getExecutablePath() {
#ifdef _WIN32
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    std::wstring wpath(buffer);
    std::string path(wpath.begin(), wpath.end());
    size_t pos = path.find_last_of("\\/");
    if (pos != std::string::npos) {
        return path.substr(0, pos + 1);
    }
    return "./";
#else
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, PATH_MAX - 1);
    if (len != -1) {
        buffer[len] = '\0';
        std::string path(buffer);
        size_t pos = path.find_last_of("/");
        if (pos != std::string::npos) {
            return path.substr(0, pos + 1);
        }
    }
    return "./";
#endif
}

// ============================================================
// 辅助函数：创建目录
// ============================================================
static void ensureDirectory(const std::string& dir) {
    if (dir.empty()) return;
#ifdef HAS_FILESYSTEM
    if (!fs::exists(dir)) {
        fs::create_directories(dir);
        std::cout << "[存档] 已创建存档目录: " << dir << std::endl;
    }
#else
    MKDIR(dir.c_str());
    std::cout << "[存档] 已创建存档目录: " << dir << std::endl;
#endif
}

// ============================================================
// 辅助函数：获取当前时间字符串（用于存档时间戳）
// ============================================================
static std::string getTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm tm_buf;
    localtime_s(&tm_buf, &now);
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// ============================================================
// 构造函数
// ============================================================

SaveManager::SaveManager() {
#ifdef PROJECT_ROOT_DIR
    std::string root = PROJECT_ROOT_DIR;
#ifdef _WIN32
    if (!root.empty() && root.back() != '/' && root.back() != '\\') {
        m_saveDir = root + "\\saves\\";
    } else {
        m_saveDir = root + "saves\\";
    }
#else
    if (!root.empty() && root.back() != '/') {
        m_saveDir = root + "/saves/";
    } else {
        m_saveDir = root + "saves/";
    }
#endif
#else
    m_saveDir = "./saves/";
#endif
    ensureDirectory(m_saveDir);
    std::cout << "[存档] 存档目录设置为: " << m_saveDir << std::endl;
}

SaveManager::SaveManager(const std::string& saveDir) : m_saveDir(saveDir) {
    // 确保路径以分隔符结尾，方便后续拼接
    if (!m_saveDir.empty() && m_saveDir.back() != '/' && m_saveDir.back() != '\\') {
        m_saveDir += "/";
    }
    ensureDirectory(m_saveDir);
}

SaveManager::~SaveManager() {
}

// ============================================================
// saveToFile — 保存游戏到文件
//
// 保存格式（文本文件，每行一个 section）：
//   [CAMPUSRPG_SAVE]
//   # 保存时间: YYYY-MM-DD HH:MM:SS
//   # 版本: 1.0
//   name|level|hp|maxHp|exp|gold|attack|defense
//   [INVENTORY]
//   类型|id|name|price|description|类型特有字段...  （每物品一行）
//   [QUESTS]
//   questId|accepted|completed|currentCount          （每任务一行）
//   [END]
// ============================================================
bool SaveManager::saveToFile(const std::string& filename,
                              const Character& character,
                              const Inventory& inventory,
                              const QuestManager& questManager) {
    std::string fullPath = m_saveDir + filename;
    std::ofstream outFile(fullPath);

    if (!outFile.is_open()) {
        std::cout << "[存档] 错误：无法打开文件 " << fullPath << " 进行写入！" << std::endl;
        return false;
    }

    // --------------------------------------------------
    // 1. 写入文件头 — 文件标记 + 时间戳 + 版本
    // --------------------------------------------------
    outFile << "[CAMPUSRPG_SAVE]" << std::endl;
    outFile << "# 保存时间: " << getTimestamp() << std::endl;
    outFile << "# 版本: 1.0" << std::endl;

    // --------------------------------------------------
    // 2. 写入角色数据 — 调用 Character::toSaveString()
    //    格式: name|level|hp|maxHp|exp|gold|attack|defense
    // --------------------------------------------------
    outFile << character.toSaveString() << std::endl;

    // --------------------------------------------------
    // 3. 写入背包数据 — 调用 Inventory::toSaveString()
    //
    // 多态体现：每种物品类型（Food/Potion/Equipment）的
    // toSaveString 实现会在行首写入类型标记，如：
    //   Food|id|name|price|desc|healAmount
    //   Potion|id|name|price|desc|effectType|statMod|turns
    //   Equipment|id|name|price|desc|slot|atkBonus|defBonus
    // --------------------------------------------------
    outFile << "[INVENTORY]" << std::endl;
    std::string invData = inventory.toSaveString();
    if (!invData.empty()) {
        outFile << invData;
        // 确保以换行结尾，避免与下一 section 粘连
        if (invData.back() != '\n') {
            outFile << std::endl;
        }
    }

    // --------------------------------------------------
    // 4. 写入任务数据 — 遍历 QuestManager 中所有任务
    //
    // 注：QuestManager 未暴露迭代器接口，故采用 ID 扫描方式
    //     遍历 [1, 100] 范围内的任务ID，逐个查询并写入
    // --------------------------------------------------
    outFile << "[QUESTS]" << std::endl;
    bool hasQuests = false;
    for (int id = 1; id <= 100; ++id) {
        const Quest* quest = questManager.findQuest(id);
        if (quest != nullptr) {
            hasQuests = true;
            outFile << quest->getId() << "|"
                    << (quest->isAccepted()  ? "1" : "0") << "|"
                    << (quest->isCompleted() ? "1" : "0") << "|"
                    << quest->getCurrentCount() << std::endl;
        }
    }
    if (!hasQuests) {
        outFile << "# (无任务数据)" << std::endl;
    }

    // --------------------------------------------------
    // 5. 写入结束标记
    // --------------------------------------------------
    outFile << "[END]" << std::endl;
    outFile.close();

    // 检查写入是否成功
    if (outFile.fail()) {
        std::cout << "[存档] 错误：写入文件 " << fullPath << " 时发生错误！" << std::endl;
        return false;
    }

    std::cout << "[存档] 游戏已成功保存到 " << fullPath << std::endl;
    return true;
}

// ============================================================
// loadFromFile — 从文件加载游戏
//
// 读取上述存档格式，按 [TAG] 逐段解析：
//   [CAMPUSRPG_SAVE] → Character::fromSaveString()
//   [INVENTORY]      → Inventory::fromSaveString()
//   [QUESTS]         → 逐行解析并更新 Quest 状态
// ============================================================
bool SaveManager::loadFromFile(const std::string& filename,
                                Character& character,
                                Inventory& inventory,
                                QuestManager& questManager) {
    std::string fullPath = m_saveDir + filename;
    std::ifstream inFile(fullPath);

    if (!inFile.is_open()) {
        std::cout << "[存档] 错误：找不到存档文件 " << fullPath << std::endl;
        return false;
    }

    std::string line;

    // --------------------------------------------------
    // 1. 验证文件头 — 必须为 [CAMPUSRPG_SAVE]
    // --------------------------------------------------
    if (!std::getline(inFile, line) || line != "[CAMPUSRPG_SAVE]") {
        std::cout << "[存档] 错误：存档文件格式无效！缺少 [CAMPUSRPG_SAVE] 标记。" << std::endl;
        return false;
    }

    // 跳过注释行（时间戳、版本等），定位到角色数据行
    while (std::getline(inFile, line)) {
        if (line.empty()) continue;
        if (line[0] == '#') continue;       // 注释行
        break;                               // 角色数据行
    }

    // --------------------------------------------------
    // 2. 读取角色数据 — 调用 Character::fromSaveString()
    // --------------------------------------------------
    if (line.empty()) {
        std::cout << "[存档] 错误：存档中缺少角色数据！" << std::endl;
        return false;
    }
    character.fromSaveString(line);
    std::cout << "[存档] 角色数据加载成功。" << std::endl;

    // --------------------------------------------------
    // 3. 读取背包数据 — 收集 [INVENTORY] 到下一 section 之间的所有行
    // --------------------------------------------------
    if (!std::getline(inFile, line) || line != "[INVENTORY]") {
        std::cout << "[存档] 错误：存档中缺少 [INVENTORY] 标记！" << std::endl;
        return false;
    }

    std::string inventoryData;
    while (std::getline(inFile, line)) {
        if (line == "[QUESTS]" || line == "[END]") {
            break;  // 到达下一 section，停止收集
        }
        // 跳过注释和空行
        if (!line.empty() && line[0] != '#') {
            inventoryData += line + "\n";
        }
    }
    if (!inventoryData.empty()) {
        inventory.fromSaveString(inventoryData);
    }
    std::cout << "[存档] 背包数据加载成功（" 
              << inventory.itemCount() << " 件物品）。" << std::endl;

    // --------------------------------------------------
    // 4. 读取任务数据 — 逐行解析并更新 QuestManager
    //
    //    格式: questId|accepted|completed|currentCount
    //    accepted  : 0=未接受, 1=已接受
    //    completed : 0=未完成, 1=已完成
    // --------------------------------------------------
    if (line == "[QUESTS]") {
        int questCount = 0;
        while (std::getline(inFile, line)) {
            if (line == "[END]") break;
            if (line.empty() || line[0] == '#') continue;

            // 按 '|' 分隔解析四个字段
            std::istringstream iss(line);
            std::string token;
            int questId = 0, accepted = 0, completed = 0, currentCount = 0;

            if (std::getline(iss, token, '|')) questId     = std::stoi(token);
            if (std::getline(iss, token, '|')) accepted    = std::stoi(token);
            if (std::getline(iss, token, '|')) completed   = std::stoi(token);
            if (std::getline(iss, token, '|')) currentCount = std::stoi(token);

            // 在 QuestManager 中查找对应任务并更新状态
            Quest* quest = questManager.mutableFindQuest(questId);
            if (quest != nullptr) {
                // 恢复接受状态
                if (accepted && !quest->isAccepted()) {
                    quest->accept();
                }
                // 恢复完成状态（调用 Quest::complete() 而非 QuestManager::completeQuest，
                // 因为奖励在保存前已发放，加载时仅需恢复状态标记）
                if (completed && !quest->isCompleted()) {
                    quest->complete();
                }
                // 恢复当前进度计数
                quest->setCurrentCount(currentCount);
                questCount++;
            } else {
                std::cout << "[存档] 警告：存档中的任务ID " << questId
                          << " 在当前任务管理器中不存在，已跳过。" << std::endl;
            }
        }
        std::cout << "[存档] 任务数据加载成功（" << questCount << " 个任务）。" << std::endl;
    }

    inFile.close();
    std::cout << "[存档] 游戏已成功从 " << fullPath << " 加载！" << std::endl;
    return true;
}

// ============================================================
// fileExists — 检查存档文件是否存在
// ============================================================
bool SaveManager::fileExists(const std::string& filename) const {
    std::string fullPath = m_saveDir + filename;
#ifdef HAS_FILESYSTEM
    return fs::exists(fullPath) && fs::is_regular_file(fullPath);
#else
    std::ifstream test(fullPath);
    return test.good();
#endif
}

// ============================================================
// listSaveFiles — 列出存档目录下所有文件
// ============================================================
void SaveManager::listSaveFiles() const {
    std::cout << "[存档] 存档目录: " << m_saveDir << std::endl;
    std::cout << "[存档] 已有存档列表:" << std::endl;

#ifdef HAS_FILESYSTEM
    if (!fs::exists(m_saveDir)) {
        std::cout << "  (存档目录不存在)" << std::endl;
        return;
    }

    int count = 0;
    for (const auto& entry : fs::directory_iterator(m_saveDir)) {
        if (entry.is_regular_file()) {
            count++;
            std::cout << "  " << count << ". " << entry.path().filename().string()
                      << "  (" << entry.file_size() << " bytes)" << std::endl;
        }
    }
    if (count == 0) {
        std::cout << "  (暂无存档文件)" << std::endl;
    } else {
        std::cout << "  共 " << count << " 个存档文件。" << std::endl;
    }
#else
    std::cout << "  (需要 C++17 filesystem 支持以遍历目录)" << std::endl;
#endif
}

// ============================================================
// Getter / Setter
// ============================================================

const std::string& SaveManager::getSaveDir() const {
    return m_saveDir;
}

void SaveManager::setSaveDir(const std::string& dir) {
    m_saveDir = dir;
    // 确保路径以分隔符结尾
    if (!m_saveDir.empty() && m_saveDir.back() != '/' && m_saveDir.back() != '\\') {
        m_saveDir += "/";
    }
    ensureDirectory(m_saveDir);
    std::cout << "[存档] 存档目录已切换为: " << m_saveDir << std::endl;
}
