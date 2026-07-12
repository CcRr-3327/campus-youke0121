#include "gamemanager.h"
#include <ctime>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef HAS_SFML
#include "gui.h"
#endif

int main() {
#ifdef _WIN32
    // 双保险：SetConsoleOutputCP + system(chcp)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    system("chcp 65001 > nul");
#endif

    srand(static_cast<unsigned>(time(nullptr)));

    GameManager& gm = GameManager::getInstance();

#ifdef HAS_SFML
    GUI gui;
    gui.run();
#else
    gm.showMainMenu();
    gm.run();
#endif

    return 0;
}
