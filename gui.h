#pragma once

#ifdef HAS_SFML

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <memory>
#include "gamemanager.h"
#include "enemy.h"
#include "map.h"

enum class GuiState {
    MainMenu, MapExploring, InBattle, InShop,
    Inventory, Quests, CharacterPanel, MenuPopup, GameOver, BuildingInterior,
    LibraryBorrow, LibraryStudy, ClassroomLab, ClassroomSubmit, SaveManager
};

enum class BattlePhase2 { PlayerTurn, EnemyTurn, BattleOver };

class GUI {
public:
    GUI(); ~GUI(); void run();
private:
    sf::RenderWindow m_window;
    sf::Font m_font; bool m_fontLoaded;
    GuiState m_state=GuiState::MainMenu;
    Map m_map;

    std::string m_inputBuffer;
    bool m_inputActive=false;
    std::string m_inputPrompt;
    std::string m_narration;
    std::string m_buildingName;

    sf::Vector2i m_playerPos;
    sf::Vector2f m_playerDrawPos;
    bool m_moving=false; float m_moveTimer=0; sf::Vector2i m_moveTarget;

    BattlePhase2 m_battlePhase = BattlePhase2::PlayerTurn;
    bool m_defending=false;
    CombatResult m_battleResult=CombatResult::None;
    std::unique_ptr<Enemy> m_battleEnemy;
    std::vector<std::string> m_battleLog;

    int m_shopBuyId=0; int m_invUseId=0;
    bool m_menuOpen=false; int m_menuSel=0;
    std::vector<std::string> m_saveFiles;
    bool m_saveMode=false;

    void handleEvents(); void update(float dt); void render();
    void renderMainMenu(); void renderMap(); void renderBattle();
    void renderShop(); void renderInventory(); void renderQuests();
    void renderCharacterPanel(); void renderMenuPopup(); void renderGameOver();
    void renderInputBox(); void renderBuildingInterior();
    void renderLibraryBorrow(); void renderLibraryStudy();
    void renderClassroomLab(); void renderClassroomSubmit();
    void renderSaveManager();

    void handleMainMenuClick(int mx,int my);
    void handleMapClick(int mx,int my);
    void handleMapKey(sf::Keyboard::Key key);
    void handleBattleClick(int mx,int my);
    void handleShopClick(int mx,int my);
    void handleInvClick(int mx,int my);
    void handleQuestClick(int mx,int my);
    void handleCharPanelClick(int mx,int my);
    void handleMenuClick(int mx,int my);
    void handleGameOverClick(int mx,int my);
    void handleBuildingClick(int mx,int my);
    void handleLibraryBorrowClick(int mx,int my);
    void handleLibraryStudyClick(int mx,int my);
    void handleClassroomLabClick(int mx,int my);
    void handleClassroomSubmitClick(int mx,int my);
    void handleSaveManagerClick(int mx,int my);
    void handleGlobalKey(sf::Keyboard::Key key);
    void processInputConfirm(const std::string& input);
    void drawNarrationBox();
    void refreshSaveFiles();

    void drawText(const std::string& text,float x,float y,unsigned size,sf::Color c=sf::Color::White);
    void drawButton(const std::string& label,float x,float y,float w,float h,bool hover);
    sf::FloatRect getButtonRect(float x,float y,float w,float h);
    void drawHPBar(float x,float y,float w,float h,int cur,int max,sf::Color fg);
    void drawStatusBar();
    void drawMiniInventory();
    void drawNarrationBar();
    void drawBattleLogPanel(float x,float y,float w,float h);
    void startNewBattle();
    void doPlayerAttack();
    void doEnemyAttack();
    void endBattle();
    void newGameGUI(const std::string& name);
    void triggerTileAction();
    void showNarration(const std::string& msg);
    void drawPixelPlayer(float cx, float cy);
    void drawPixelEnemy(float cx, float cy,const std::string& type);
};

#endif
