#pragma once

#include "Item.h"

class Book : public Item {
public:
    Book();
    Book(int id, const std::string& name, int price,
         const std::string& description, int expBonus);

    int getExpBonus() const;
    void setExpBonus(int bonus);

private:
    int m_expBonus;
};