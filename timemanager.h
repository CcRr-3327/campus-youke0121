#include <string>
#include <vector>
using namespace std;

class Student;
class Narrator;

struct DayInfo {
    int dayOfWeek;      // 1-7
    string dayName;     // "周一" ~ "周日"
    bool isToday;
    bool hasExam;
    string event;
};

class TimeManager {
private:
 int dayOfWeek;      // 1=周一, 5=周五, 7=周日
    int weekCount;
    int totalDays;
    int lastWeekExp;    // 上周经验，用于计算本周考试难度

public:
    TimeManager();

    int getDayOfWeek() const { return dayOfWeek; }
    int getWeekCount() const { return weekCount; }
    int getTotalDays() const { return totalDays; }
    int getLastWeekExp() const { return lastWeekExp; }

    string getDayName(int day) const;
    string getTodayName() const;
    bool isFriday() const { return dayOfWeek == 5; }
    bool isSunday() const { return dayOfWeek == 7; }
    bool isWeekend() const { return dayOfWeek == 6 || dayOfWeek == 7; }

    void nextDay(Student* student, Narrator* narrator);
    void onNewWeek(Student* student, Narrator* narrator);
    void onSundayEvent(Student* student, Narrator* narrator);
    void onFridayExam(Student* student, Narrator* narrator);

    void showCalendar() const;
    vector<DayInfo> getWeekData() const;

    void serialize(ostream& out) const;
    void deserialize(istream& in);
};