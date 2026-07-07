

#include <string>
#include <iostream>
using namespace std;

class Student;

enum ItemType {
    FOOD,
    DRINK,
    BOOK,
    TOOL
};

class Item {
protected:
    int id;
    string name;
    ItemType type;
    int price;
    string description;
    int effectValue;

public:
    Item(int i, string n, ItemType t, int p, string d, int ev)
        : id(i), name(n), type(t), price(p), description(d), effectValue(ev) {}
    virtual ~Item() {}

    int getId() const { return id; }
    string getName() const { return name; }
    ItemType getType() const { return type; }
    int getPrice() const { return price; }
    string getDescription() const { return description; }
    int getEffectValue() const { return effectValue; }

    virtual void use(Student* student) = 0;
    virtual string getTypeString() const = 0;
    virtual void showInfo() const;
};

class FoodItem : public Item {
public:
    FoodItem(int i=0, string n="", int p=0, string d="", int ev=0)
        : Item(i, n, FOOD, p, d, ev) {}
    void use(Student* student) override;
    string getTypeString() const override { return "食物"; }
};

class DrinkItem : public Item {
public:
    DrinkItem(int i=0, string n="", int p=0, string d="", int ev=0)
        : Item(i, n, DRINK, p, d, ev) {}
    void use(Student* student) override;
    string getTypeString() const override { return "饮料"; }
};

class BookItem : public Item {
public:
    BookItem(int i=0, string n="", int p=0, string d="", int ev=0)
        : Item(i, n, BOOK, p, d, ev) {}
    void use(Student* student) override;
    string getTypeString() const override { return "书籍"; }
};

class ToolItem : public Item {
public:
    ToolItem(int i=0, string n="", int p=0, string d="", int ev=0)
        : Item(i, n, TOOL, p, d, ev) {}
    void use(Student* student) override;
    string getTypeString() const override { return "工具"; }
};

Item* createItem(int type, int id, string name, int price, string desc, int effect);

