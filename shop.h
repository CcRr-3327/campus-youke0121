#pragma once

#include "Item.h"
#include <map>
#include <memory>
#include <string>

class Character;
class Inventory;

/**
 * Shop — 商店类
 *
 * 使用 std::map<int, std::shared_ptr<Item>> 按商品 ID 索引管理。
 * 支持购买和出售。
 */
class Shop {
public:
    Shop();
    ~Shop();

    // ---- 商店操作 ----
    void showItems() const;                                 // 展示商品列表
    bool buyItem(int id, Character& character);             // 购买（扣金币，加入背包）
    void sellItem(int id, Character& character, Inventory& inventory);  // 出售（背包移除，加金币）

    // ---- 商品管理 ----
    void addItem(std::shared_ptr<Item> item);               // 添加商品到商店
    void removeItem(int id);                                // 移除商品
    std::shared_ptr<Item> findItem(int id) const;           // 查找商品

private:
    std::map<int, std::shared_ptr<Item>> m_items;           // 商品按 ID 索引
};
