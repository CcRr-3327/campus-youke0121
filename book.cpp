#include "book.h"
#include "Character.h"
#include <sstream>

Book::Book()
    : Item(), m_expBonus(0)
{
}

Book::Book(int id, const std::string& name, int price,
           const std::string& description, int expBonus)
    : Item(id, name, "书籍", price, description), m_expBonus(expBonus)
{
}

void Book::use(Character& character) {
    character.gainExp(m_expBonus);
}

std::string Book::getInfo() const {
    std::ostringstream oss;
    oss << m_id << "|" << m_name << "|书籍|" << m_price << "|" << m_description << "|" << m_expBonus;
    return oss.str();
}

int Book::getExpBonus() const {
    return m_expBonus;
}

void Book::setExpBonus(int bonus) {
    m_expBonus = bonus;
}