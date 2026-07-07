#include <string>
#include <vector>
#include <map>
#include <random>
#include <iostream>
using namespace std;

class Student;

enum NarratorEvent {
    EAT_BREAKFAST,
    EAT_LUNCH,
    EAT_DINNER,
    ATTEND_CLASS,
    STUDY_LIBRARY,
    EXERCISE,
    LISTEN_MUSIC,
    SHOP_BUY,
    EXAM_START,
    EXAM_PASS,
    EXAM_FAIL,
    SUNDAY_MONEY,
    SLEEP,
    HP_ZERO,
    MOOD_ZERO,
    WAKE_UP,
    ENTER_SHOP
};

class Narrator {
private:
    static Narrator* instance;
    bool isEnabled;
    mt19937 rng;
    map<NarratorEvent, vector<string>> dialogueBank;

    Narrator();
    void initDialogueBank();
    string randomLine(const vector<string>& lines);

public:
    static Narrator* getInstance();
    void setEnabled(bool e) { isEnabled = e; }

    void speak(string text);
    void narrate(NarratorEvent event, Student* student = nullptr);

    void onEat(Student* s, string mealType);
    void onClass(Student* s);
    void onStudy(Student* s);
    void onExercise(Student* s);
    void onMusic(Student* s);
    void onShop(Student* s);
    void onExamStart(Student* s, int diff);
    void onExamPass(Student* s, int score, int diff);
    void onExamFail(Student* s, int score, int diff);
    void onSundayMoney(Student* s, int amount);
    void onSleep(Student* s);
    void onHpZero(Student* s);
    void onMoodZero(Student* s);
    void onWakeUp(Student* s);
};
