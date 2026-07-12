#include "homework.h"
#include "Character.h"
#include <sstream>

Homework::Homework()
    : Item(200, "作业", "任务道具", 0, "未完成的作业")
{
}

Homework::Homework(int id)
    : Item(id, "作业", "任务道具", 0, "未完成的作业")
{
}

void Homework::use(Character& character) {
}

std::string Homework::getInfo() const {
    std::ostringstream oss;
    oss << m_id << "|" << m_name << "|任务道具|" << m_price << "|" << m_description;
    return oss.str();
}