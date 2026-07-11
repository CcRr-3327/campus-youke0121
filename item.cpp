#include "item.h"
#include <sstream>

// ---- 构造 / 析构 ----

Item::Item()
    : m_id(0), m_name(""), m_type(""), m_price(0), m_description("")
{
}

Item::Item(int id, const std::string& name, const std::string& type,
           int price, const std::string& description)
    : m_id(id), m_name(name), m_type(type), m_price(price), m_description(description)
{
}

Item::~Item()
{
}

// ---- 多态接口 ----

std::string Item::getInfo() const
{
    // 格式：id|name|type|price|description
    std::ostringstream oss;
    oss << m_id << "|" << m_name << "|" << m_type << "|" << m_price << "|" << m_description;
    return oss.str();
}

// ---- Getter / Setter ----

int Item::getId() const
{
    return m_id;
}

const std::string& Item::getName() const
{
    return m_name;
}

const std::string& Item::getType() const
{
    return m_type;
}

int Item::getPrice() const
{
    return m_price;
}

const std::string& Item::getDescription() const
{
    return m_description;
}

void Item::setId(int id)
{
    m_id = id;
}

void Item::setName(const std::string& name)
{
    m_name = name;
}

void Item::setType(const std::string& type)
{
    m_type = type;
}

void Item::setPrice(int price)
{
    m_price = price;
}

void Item::setDescription(const std::string& description)
{
    m_description = description;
}
