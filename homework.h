#pragma once
#include "Item.h"
class Homework : public Item {
public:
    Homework();
    Homework(int id);
    void use(Character& character) override;
    std::string getInfo() const override;
};