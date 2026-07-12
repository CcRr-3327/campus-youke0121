#include "shop.h"
#include "character.h"
#include "inventory.h"
#include <iostream>

// ---- 构造 / 析构 ----

Shop::Shop()
{
}

Shop::~Shop()
{
}

// ---- 商店操作 ----

void Shop::showItems() const
{
    if (m_items.empty()) {
        std::cout << "商店暂时没有商品。" << std::endl;
        return;
    }

    std::cout << "===== 商店商品 =====" << std::endl;
    for (const auto& pair : m_items) {
        const auto& item = pair.second;
        std::cout << "ID:" << item->getId() << " | " << item->getName()
                  << " | 价格：" << item->getPrice() << "金币"
                  << " | " << item->getDescription() << std::endl;
    }
    std::cout << "====================" << std::endl;
}

bool Shop::buyItem(int id, Character& character)
{
    auto it = m_items.find(id);
    if (it == m_items.end()) {
        std::cout << "商店中没有此商品！" << std::endl;
        return false;
    }

    const auto& item = it->second;
    int price = item->getPrice();

    if (character.getGold() < price) {
        std::cout << "金币不足！需要" << price << "金币，当前拥有" 
                  << character.getGold() << "金币。" << std::endl;
        return false;
    }

    // 扣金币
    character.setGold(character.getGold() - price);
    std::cout << "购买了[" << item->getName() << "]！花费" << price << "金币。" << std::endl;
    return true;
}

void Shop::sellItem(int id, Character& character, Inventory& inventory)
{
    auto it = m_items.find(id);
    if (it == m_items.end()) {
        std::cout << "商店不回收此物品。" << std::endl;
        return;
    }

    const auto& item = it->second;

    // 检查背包中是否拥有此物品
    auto ownedItem = inventory.findItem(id);
    if (!ownedItem) {
        std::cout << "你的背包中没有此物品！" << std::endl;
        return;
    }

    int sellPrice = item->getPrice() * 60 / 100;  // 售价为原价的60%

    // 从背包移除物品
    inventory.removeItem(id);

    // 增加金币
    character.setGold(character.getGold() + sellPrice);
    std::cout << "出售了[" << item->getName() << "]，获得" << sellPrice << "金币。" << std::endl;
}

// ---- 商品管理 ----

void Shop::addItem(std::shared_ptr<Item> item)
{
    if (!item) return;
    m_items[item->getId()] = item;
}

void Shop::removeItem(int id)
{
    auto it = m_items.find(id);
    if (it != m_items.end()) {
        std::cout << "商品 [" << it->second->getName() << "] 已下架。" << std::endl;
        m_items.erase(it);
    }
}

std::shared_ptr<Item> Shop::findItem(int id) const
{
    auto it = m_items.find(id);
    if (it != m_items.end()) {
        return it->second;
    }
    return nullptr;
}
