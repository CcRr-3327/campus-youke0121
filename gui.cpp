#ifdef HAS_SFML

#include "gui.h"
#include "slime.h"
#include "goblin.h"
#include "boss.h"
#include "book.h"
#include "homework.h"
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <algorithm>
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#define HAS_FILESYSTEM 1
#else
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define MKDIR(p) _mkdir(p)
#else
#include <sys/stat.h>
#include <dirent.h>
#define MKDIR(p) mkdir(p, 0755)
#endif
#endif

static const sf::Color COLOR_BG(0x1a,0x1a,0x2e), COLOR_CARD(0x16,0x21,0x3e);
static const sf::Color COLOR_HIGHLIGHT(0x0f,0x34,0x60), COLOR_OUTLINE(0x53,0x5b,0x7a);
static const sf::Color COLOR_GOLD(0xff,0xd7,0), COLOR_GREEN(0x2e,0xcc,0x40);
static const sf::Color COLOR_RED(0xe7,0x4c,0x3c), COLOR_BLUE(0x34,0x98,0xdb);
static const sf::Color COLOR_YELLOW(0xf3,0x9c,0x12), COLOR_GREY(0x95,0xa5,0xa6), COLOR_WHITE(0xff,0xff,0xff);

static const char* FONT_CANDIDATES[] = {
    "font.ttf","C:/Windows/Fonts/simhei.ttf","C:/Windows/Fonts/msyh.ttc",
    "C:/Windows/Fonts/simsun.ttc","C:\\Windows\\Fonts\\simhei.ttf","C:\\Windows\\Fonts\\msyh.ttc",
    "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf","/System/Library/Fonts/PingFang.ttc",nullptr
};

GUI::GUI() : m_window(sf::VideoMode(800,600),L"校园RPG冒险"), m_fontLoaded(false)
{
    m_window.setFramerateLimit(60);
    for(int i=0;FONT_CANDIDATES[i];++i) if(m_font.loadFromFile(FONT_CANDIDATES[i])){m_fontLoaded=true;break;}
    if(!m_fontLoaded){
        const char* chineseFonts[] = {
            "C:/Windows/Fonts/msyh.ttc",
            "C:/Windows/Fonts/simhei.ttf",
            "C:/Windows/Fonts/simsun.ttc",
            "C:/Windows/Fonts/msyhbd.ttc",
            "C:/Windows/Fonts/msyhl.ttc",
            nullptr
        };
        for(int i=0;chineseFonts[i];++i) if(m_font.loadFromFile(chineseFonts[i])){m_fontLoaded=true;break;}
    }
    m_map.loadCampusMap();
    m_playerPos = m_map.getPlayerStart();
    m_playerDrawPos = sf::Vector2f((float)m_playerPos.x*m_map.getTileSize(),(float)m_playerPos.y*m_map.getTileSize());
}

GUI::~GUI() { if(m_window.isOpen()) m_window.close(); }

// ===== RUN =====
void GUI::run() {
    sf::Clock clock;
    while(m_window.isOpen()){
        float dt = clock.restart().asSeconds();
        handleEvents();
        update(dt);
        render();
    }
}

// ===== EVENTS =====
void GUI::handleEvents() {
    sf::Event event;
    while(m_window.pollEvent(event)){
        if(event.type==sf::Event::Closed){ m_window.close(); return; }
        if(m_inputActive && event.type==sf::Event::TextEntered){
            if(event.text.unicode==13){ std::string s=m_inputBuffer; m_inputActive=false; m_inputBuffer.clear(); processInputConfirm(s); return; }
            if(event.text.unicode==8){ if(!m_inputBuffer.empty())m_inputBuffer.pop_back(); return; }
            sf::Uint32 cp=event.text.unicode;
            if(cp>=32&&cp!=127){
                if(cp<0x80) m_inputBuffer+=static_cast<char>(cp);
                else if(cp<0x800){ m_inputBuffer+=static_cast<char>(0xC0|(cp>>6)); m_inputBuffer+=static_cast<char>(0x80|(cp&0x3F)); }
                else if(cp<0x10000){ m_inputBuffer+=static_cast<char>(0xE0|(cp>>12)); m_inputBuffer+=static_cast<char>(0x80|((cp>>6)&0x3F)); m_inputBuffer+=static_cast<char>(0x80|(cp&0x3F)); }
                else { m_inputBuffer+=static_cast<char>(0xF0|(cp>>18)); m_inputBuffer+=static_cast<char>(0x80|((cp>>12)&0x3F)); m_inputBuffer+=static_cast<char>(0x80|((cp>>6)&0x3F)); m_inputBuffer+=static_cast<char>(0x80|(cp&0x3F)); }
            }
            continue;
        }
        if(event.type==sf::Event::KeyPressed){
            handleGlobalKey(event.key.code);
            if(m_state==GuiState::MapExploring&&!m_moving) handleMapKey(event.key.code);
        }
        if(event.type==sf::Event::MouseButtonPressed&&event.mouseButton.button==sf::Mouse::Left){
            int mx=event.mouseButton.x, my=event.mouseButton.y;
            if(m_menuOpen){ handleMenuClick(mx,my); return; }
            switch(m_state){
            case GuiState::MainMenu: handleMainMenuClick(mx,my); break;
            case GuiState::MapExploring: handleMapClick(mx,my); break;
            case GuiState::InBattle: handleBattleClick(mx,my); break;
            case GuiState::InShop: handleShopClick(mx,my); break;
            case GuiState::Inventory: handleInvClick(mx,my); break;
            case GuiState::Quests: handleQuestClick(mx,my); break;
            case GuiState::CharacterPanel: handleCharPanelClick(mx,my); break;
            case GuiState::MenuPopup: handleMenuClick(mx,my); break;
            case GuiState::GameOver: handleGameOverClick(mx,my); break;
            case GuiState::BuildingInterior: handleBuildingClick(mx,my); break;
            case GuiState::LibraryBorrow: handleLibraryBorrowClick(mx,my); break;
            case GuiState::LibraryStudy: handleLibraryStudyClick(mx,my); break;
            case GuiState::ClassroomLab: handleClassroomLabClick(mx,my); break;
            case GuiState::ClassroomSubmit: handleClassroomSubmitClick(mx,my); break;
            case GuiState::SaveManager: handleSaveManagerClick(mx,my); break;
            }
        }
    }
}

void GUI::handleGlobalKey(sf::Keyboard::Key key) {
    if(m_inputActive) return;
    if(key==sf::Keyboard::Escape){
        if(m_state==GuiState::BuildingInterior){ m_state=GuiState::MapExploring; return; }
        if(m_state==GuiState::LibraryBorrow||m_state==GuiState::LibraryStudy||
           m_state==GuiState::ClassroomLab||m_state==GuiState::ClassroomSubmit){
            m_state=GuiState::BuildingInterior; return;
        }
        if(m_state==GuiState::MapExploring){ m_menuOpen=!m_menuOpen; m_menuSel=0; }
        else if(m_state!=GuiState::MainMenu&&m_state!=GuiState::GameOver) m_state=GuiState::MapExploring;
    }
}

// ===== UPDATE =====
void GUI::update(float dt) {
    if(m_moving){
        m_moveTimer+=dt*6.f;
        if(m_moveTimer>=1.f){ m_moveTimer=0; m_moving=false; m_playerDrawPos=sf::Vector2f((float)m_moveTarget.x*m_map.getTileSize(),(float)m_moveTarget.y*m_map.getTileSize()); }
        else {
            float t=m_moveTimer;
            m_playerDrawPos=sf::Vector2f(
                (1-t)*m_playerPos.x*m_map.getTileSize()+t*m_moveTarget.x*m_map.getTileSize(),
                (1-t)*m_playerPos.y*m_map.getTileSize()+t*m_moveTarget.y*m_map.getTileSize());
        }
    }
    // 战斗敌人回合自动
    if(m_state==GuiState::InBattle&&m_battlePhase==BattlePhase2::EnemyTurn&&m_battleEnemy){
        doEnemyAttack();
        if(m_battlePhase==BattlePhase2::BattleOver) endBattle();
        else m_battlePhase=BattlePhase2::PlayerTurn;
    }
}

// ===== RENDER =====
void GUI::render() {
    m_window.clear(COLOR_BG);
    switch(m_state){
    case GuiState::MainMenu: renderMainMenu(); break;
    case GuiState::MapExploring: renderMap(); if(m_menuOpen)renderMenuPopup(); break;
    case GuiState::InBattle: renderBattle(); break;
    case GuiState::InShop: renderShop(); break;
    case GuiState::Inventory: renderInventory(); break;
    case GuiState::Quests: renderQuests(); break;
    case GuiState::CharacterPanel: renderCharacterPanel(); break;
    case GuiState::GameOver: renderGameOver(); break;
    case GuiState::BuildingInterior: renderBuildingInterior(); break;
    case GuiState::LibraryBorrow: renderLibraryBorrow(); break;
    case GuiState::LibraryStudy: renderLibraryStudy(); break;
    case GuiState::ClassroomLab: renderClassroomLab(); break;
    case GuiState::ClassroomSubmit: renderClassroomSubmit(); break;
    case GuiState::SaveManager: renderSaveManager(); break;
    }
    if(m_inputActive&&m_state!=GuiState::MainMenu) renderInputBox();
    m_window.display();
}

// ===== MAIN MENU =====
void GUI::renderMainMenu() {
    drawText("校园RPG冒险",400,100,48,COLOR_GOLD);
    drawText("Campus RPG Adventure",400,160,18,COLOR_OUTLINE);
    sf::Vector2i m=sf::Mouse::getPosition(m_window);
    drawButton("开始新游戏",300,230,200,44,getButtonRect(300,230,200,44).contains((float)m.x,(float)m.y));
    drawButton("读取存档",300,290,200,44,getButtonRect(300,290,200,44).contains((float)m.x,(float)m.y));
    drawButton("退出游戏",300,350,200,44,getButtonRect(300,350,200,44).contains((float)m.x,(float)m.y));
    if(m_inputActive) renderInputBox();
}

void GUI::handleMainMenuClick(int mx, int my) {
    if(getButtonRect(300,230,200,44).contains((float)mx,(float)my)){ m_inputActive=true; m_inputPrompt="请输入角色名称"; m_inputBuffer.clear(); }
    else if(getButtonRect(300,290,200,44).contains((float)mx,(float)my)){ m_saveMode=false; refreshSaveFiles(); m_state=GuiState::SaveManager; }
    else if(getButtonRect(300,350,200,44).contains((float)mx,(float)my)) m_window.close();
}

// ===== MAP =====
void GUI::renderMap() {
    m_map.draw(m_window,m_font);
    // 绘制像素角色 — 校服学生
    int ts=m_map.getTileSize();
    float cx=m_playerDrawPos.x+ts/2.f, cy=m_playerDrawPos.y+ts/2.f;
    float s=ts*0.32f;
    // 腿
    sf::RectangleShape leg(sf::Vector2f(s*0.35f,s*0.6f));
    leg.setFillColor(sf::Color(0x4a,0x4a,0x4a));
    leg.setPosition(cx-s*0.35f,cy+s*0.3f); m_window.draw(leg);
    leg.setPosition(cx+s*0.02f,cy+s*0.3f); m_window.draw(leg);
    // 身体（校服蓝）
    sf::RectangleShape body(sf::Vector2f(s*0.9f,s*0.8f));
    body.setFillColor(sf::Color(0x3a,0x7c,0xbf));
    body.setOutlineColor(sf::Color(0x1a,0x4a,0x7a)); body.setOutlineThickness(1);
    body.setOrigin(body.getSize().x/2.f,0); body.setPosition(cx,cy-s*0.1f); m_window.draw(body);
    // 头
    sf::CircleShape head(s*0.38f);
    head.setFillColor(sf::Color(0xff,0xe0,0xbd));
    head.setOutlineColor(sf::Color(0xd4,0xa0,0x7a)); head.setOutlineThickness(1);
    head.setOrigin(s*0.38f,s*0.38f); head.setPosition(cx,cy-s*0.5f); m_window.draw(head);
    // 头发
    sf::RectangleShape hair(sf::Vector2f(s*0.8f,s*0.12f));
    hair.setFillColor(sf::Color(0x2a,0x2a,0x2a));
    hair.setPosition(cx-s*0.4f,cy-s*0.88f); m_window.draw(hair);
    // 眼睛
    sf::CircleShape eye(2); eye.setFillColor(sf::Color::Black);
    eye.setPosition(cx-3.5f,cy-s*0.55f); m_window.draw(eye);
    eye.setPosition(cx+1.5f,cy-s*0.55f); m_window.draw(eye);
    // 名字
    auto& ch=GameManager::getInstance().getCharacter();
    drawText(ch.getName(), cx, cy-s*1.5f-10, 11, COLOR_GOLD);
    // 状态栏
    drawStatusBar();
    // 迷你背包
    drawMiniInventory();
    if(m_inputActive) renderInputBox();
}

void GUI::drawStatusBar() {
    sf::RectangleShape bar(sf::Vector2f(800,28));
    bar.setFillColor(sf::Color(0,0,0,180));
    bar.setPosition(0,572);
    m_window.draw(bar);
    auto& ch=GameManager::getInstance().getCharacter();
    std::ostringstream ss;
    ss<<ch.getName()<<"  等级."<<ch.getLevel()<<"  生命:"<<ch.getHp()<<"/"<<ch.getMaxHp()<<"  金币:"<<ch.getGold()<<"  攻击:"<<ch.getAttack()<<"  防御:"<<ch.getDefense();
    drawText(ss.str(),400,586,13,COLOR_WHITE);
}

void GUI::drawMiniInventory() {
    sf::Vector2i m=sf::Mouse::getPosition(m_window);
    auto& inv=GameManager::getInstance().getInventory();
    std::string label="背包 ["+std::to_string(inv.itemCount())+"/20]";
    drawButton(label, 600, 10, 180, 40, getButtonRect(600,10,180,40).contains((float)m.x,(float)m.y));
}

void GUI::handleMapKey(sf::Keyboard::Key key) {
    int dx=0,dy=0;
    if(key==sf::Keyboard::W||key==sf::Keyboard::Up) dy=-1;
    else if(key==sf::Keyboard::S||key==sf::Keyboard::Down) dy=1;
    else if(key==sf::Keyboard::A||key==sf::Keyboard::Left) dx=-1;
    else if(key==sf::Keyboard::D||key==sf::Keyboard::Right) dx=1;
    else return;
    int nx=m_playerPos.x+dx, ny=m_playerPos.y+dy;
    if(m_map.isWalkable(nx,ny)){
        m_moveTarget=sf::Vector2i(nx,ny);
        m_moving=true;
        m_moveTimer=0;
        m_playerPos=m_moveTarget;
        const auto& t=m_map.getTile(nx,ny);
        if(t.type==5) triggerTileAction();
    }
}

void GUI::handleMapClick(int mx, int my) {
    if(getButtonRect(600,10,180,40).contains((float)mx,(float)my)){
        m_state=GuiState::Inventory;
        return;
    }
    int ts=m_map.getTileSize();
    int tx=mx/ts, ty=my/ts;
    if(m_map.isWalkable(tx,ty)){
        m_playerPos=sf::Vector2i(tx,ty);
        m_playerDrawPos=sf::Vector2f((float)tx*ts,(float)ty*ts);
        const auto& t=m_map.getTile(tx,ty);
        if(t.type==5) triggerTileAction();
    }
}
void GUI::newGameGUI(const std::string& name) {
    auto& ch=GameManager::getInstance().getCharacter();
    ch.setName(name); ch.setLevel(1); ch.setHp(100); ch.setMaxHp(100); ch.setExp(0); ch.setGold(50); ch.setAttack(10); ch.setDefense(5);
    GameManager::getInstance().setState(GameState::Exploring);
    m_state=GuiState::MapExploring;
}

void GUI::triggerTileAction() {
    const auto& t=m_map.getTile(m_playerPos.x,m_playerPos.y);
    m_buildingName=t.label;
    // 校门口
    if(t.label=="校门口"){ m_state=GuiState::CharacterPanel; return; }
    // 其余五栋建筑进内部场景
    m_state=GuiState::BuildingInterior;
    // 设置一条随机幽默旁白
    const char* quips[]={
        "欢迎光临！请随手关门，或者不关也行。",
        "在这栋楼里，每个人都在假装很忙。你也试试。",
        "墙上贴着一张泛黄的通知：「禁止内卷」。下面有人用红笔写着「已卷」。",
        "空调温度刚好——就是说，夏天像冬天，冬天像夏天。",
        "楼道的灯闪烁了两下，它可能也在思考人生。",
        "远处传来老师讲课的声音……也可能是白噪音APP。",
        "温馨提示：本楼WiFi密码是八个8。当然这是骗你的。",
        "保洁阿姨刚刚拖完地，地面光滑得像你期末考试的大脑。",
        "楼梯间的回声在说：快~去~学~习~。其实是你幻听了。",
        "角落里有个饮水机，上面贴着'请勿投币'。谁会在饮水机上投币？"
    };
    m_battleLog.clear(); m_battleLog.push_back(quips[rand()%10]);
    // 随机触发战斗
    if(rand()%100<30) startNewBattle();
}

// ===== BATTLE =====
void GUI::startNewBattle() {
    int r=rand()%3;
    if(r==0) m_battleEnemy=std::make_unique<Slime>("校园史莱姆",std::max(1,GameManager::getInstance().getCharacter().getLevel()));
    else if(r==1) m_battleEnemy=std::make_unique<Goblin>("逃课哥布林",std::max(1,GameManager::getInstance().getCharacter().getLevel()));
    else m_battleEnemy=std::make_unique<Boss>("教导主任",std::max(1,GameManager::getInstance().getCharacter().getLevel()),"教务处");
    m_battlePhase=BattlePhase2::PlayerTurn;
    m_battleResult=CombatResult::None;
    m_battleLog.clear();
    m_defending=false;
    m_battleLog.push_back("遭遇了 "+m_battleEnemy->getName()+"！");
    m_state=GuiState::InBattle;
}

void GUI::renderBattle() {
    if(!m_battleEnemy){ m_state=GuiState::MapExploring; return; }
    auto& ch=GameManager::getInstance().getCharacter();
    Enemy* e=m_battleEnemy.get();
    // 背景遮罩
    sf::RectangleShape overlay(sf::Vector2f(800,600));
    overlay.setFillColor(sf::Color(0,0,0,120)); m_window.draw(overlay);
    // 敌人区
    sf::RectangleShape p1(sf::Vector2f(350,120)); p1.setPosition(30,30); p1.setFillColor(COLOR_CARD); p1.setOutlineColor(COLOR_OUTLINE); p1.setOutlineThickness(1); m_window.draw(p1);
    drawText(e->getName(),50,45,20,COLOR_RED);
    drawHPBar(50,70,310,18,e->getHp(),e->getMaxHp(),COLOR_RED);
    drawText(std::to_string(e->getHp())+"/"+std::to_string(e->getMaxHp()),360,78,12,COLOR_WHITE);
    // 玩家区
    sf::RectangleShape p2(sf::Vector2f(350,100)); p2.setPosition(30,170); p2.setFillColor(COLOR_CARD); p2.setOutlineColor(COLOR_OUTLINE); p2.setOutlineThickness(1); m_window.draw(p2);
    drawText(ch.getName()+" 等级"+std::to_string(ch.getLevel()),50,185,18,COLOR_WHITE);
    drawHPBar(50,210,310,18,ch.getHp(),ch.getMaxHp(),COLOR_GREEN);
    drawText(std::to_string(ch.getHp())+"/"+std::to_string(ch.getMaxHp()),360,218,12,COLOR_WHITE);
    // 日志
    drawBattleLogPanel(420,30,350,240);
    // 按钮
    sf::Vector2i m=sf::Mouse::getPosition(m_window);
    if(m_battlePhase==BattlePhase2::PlayerTurn){
        drawButton("攻击",50,300,150,40,getButtonRect(50,300,150,40).contains((float)m.x,(float)m.y));
        drawButton("防御",230,300,150,40,getButtonRect(230,300,150,40).contains((float)m.x,(float)m.y));
        drawButton("逃跑",410,300,150,40,getButtonRect(410,300,150,40).contains((float)m.x,(float)m.y));
    }
    drawText(m_battlePhase==BattlePhase2::PlayerTurn?"轮到你了":"敌人行动中...",400,360,14,COLOR_YELLOW);
}

void GUI::drawBattleLogPanel(float x, float y, float w, float h) {
    sf::RectangleShape bg(sf::Vector2f(w,h)); bg.setPosition(x,y); bg.setFillColor(COLOR_CARD); bg.setOutlineColor(COLOR_OUTLINE); bg.setOutlineThickness(1); m_window.draw(bg);
    drawText("战斗日志",x+w/2,y+10,14,COLOR_GOLD);
    int start=std::max(0,(int)m_battleLog.size()-6);
    for(size_t i=start;i<m_battleLog.size();++i)
        drawText(m_battleLog[i],x+w/2,y+34+(i-start)*24,11,COLOR_GREY);
}

void GUI::handleBattleClick(int mx, int my) {
    if(m_battlePhase!=BattlePhase2::PlayerTurn) return;
    if(getButtonRect(50,300,150,40).contains((float)mx,(float)my)){ // 攻击
        doPlayerAttack();
        if(m_battlePhase==BattlePhase2::BattleOver){ endBattle(); return; }
        m_battlePhase=BattlePhase2::EnemyTurn;
    }
    else if(getButtonRect(230,300,150,40).contains((float)mx,(float)my)){ // 防御
        m_defending=true; m_battleLog.push_back("进入防御姿态");
        m_battlePhase=BattlePhase2::EnemyTurn;
    }
    else if(getButtonRect(410,300,150,40).contains((float)mx,(float)my)){ // 逃跑
        if(rand()%100<50){ m_battleLog.push_back("成功逃跑！"); endBattle(); }
        else { m_battleLog.push_back("逃跑失败"); m_battlePhase=BattlePhase2::EnemyTurn; }
    }
}

void GUI::doPlayerAttack() {
    auto& ch=GameManager::getInstance().getCharacter();
    Enemy* e=m_battleEnemy.get();
    int dmg=std::max(1,ch.getAttack()+rand()%5);
    e->takeDamage(dmg);
    const char* thoughts[]={
        "我必须打败这个敌人！",
        "看招！",
        "拼了！",
        "这一击很关键！",
        "绝不认输！"
    };
    m_battleLog.push_back("【内心】"+std::string(thoughts[rand()%5])+" 攻击了 "+e->getName()+"，造成 "+std::to_string(dmg)+" 伤害");
    if(!e->isAlive()){ m_battleResult=CombatResult::PlayerWon; m_battlePhase=BattlePhase2::BattleOver; }
}

void GUI::doEnemyAttack() {
    auto& ch=GameManager::getInstance().getCharacter();
    Enemy* e=m_battleEnemy.get();
    int dmg=e->attack(ch);
    if(m_defending){ dmg/=2; m_defending=false; m_battleLog.push_back("防御减半！"); }
    ch.takeDamage(dmg);
    std::string msg=e->getName()+e->getLastAttackMessage()+" 造成 "+std::to_string(dmg)+" 伤害";
    if(e->getLastAttackMessage().empty()) msg=e->getName()+" 攻击了，造成 "+std::to_string(dmg)+" 伤害";
    m_battleLog.push_back(msg);
    if(!ch.isAlive()){ m_battleResult=CombatResult::PlayerLost; m_battlePhase=BattlePhase2::BattleOver; }
}

void GUI::endBattle() {
    auto& gm=GameManager::getInstance();
    auto& ch=gm.getCharacter();
    auto& qm=gm.getQuestManager();
    auto& inv=gm.getInventory();
    Enemy* e=m_battleEnemy.get();
    if(e&&m_battleResult==CombatResult::PlayerWon){
        ch.gainExp(e->getExpReward());
        ch.gainGold(e->getGoldReward());
        m_battleLog.push_back("胜利！获得 "+std::to_string(e->getExpReward())+"EXP +"+std::to_string(e->getGoldReward())+"金币");

        std::string enemyName=e->getName();
        if(enemyName.find("史莱姆")!=std::string::npos){
            qm.updateProgress(1,1);
            m_battleLog.push_back("任务进度:击败史莱姆 +1");
            if(rand()%5==0){
                inv.addItem(std::make_shared<Homework>(100+rand()%10));
                m_battleLog.push_back("获得作业本！");
                qm.updateProgress(4,1);
            }
        }else if(enemyName.find("哥布林")!=std::string::npos){
            qm.updateProgress(2,1);
            m_battleLog.push_back("任务进度:击败哥布林 +1");
        }else if(enemyName.find("主任")!=std::string::npos||enemyName.find("Boss")!=std::string::npos){
            qm.updateProgress(3,1);
            m_battleLog.push_back("任务进度:击败Boss +1");
        }
    }
    if(m_battleResult==CombatResult::PlayerLost) m_state=GuiState::GameOver;
    else m_state=GuiState::MapExploring;
    m_battleEnemy.reset();
}

// ===== SHOP =====
void GUI::renderShop() {
    sf::RectangleShape bg(sf::Vector2f(780,560)); bg.setPosition(10,10); bg.setFillColor(COLOR_CARD); bg.setOutlineColor(COLOR_OUTLINE); bg.setOutlineThickness(1); m_window.draw(bg);
    auto& sh=GameManager::getInstance().getShop();
    auto& ch=GameManager::getInstance().getCharacter();
    drawText("校园商店 — 金币:"+std::to_string(ch.getGold()),400,30,20,COLOR_GOLD);
    drawText("(点击购买物品ID)",400,55,12,COLOR_GREY);
    // 简化：直接列表 + 按钮
    float y=90;
    for(int id=1;id<=8;++id){
        auto item=sh.findItem(id);
        if(!item) continue;
        std::ostringstream ss; ss<<"ID:"<<id<<" "<<item->getName()<<"  "<<item->getPrice()<<"金币 | "<<item->getDescription();
        sf::Vector2i m=sf::Mouse::getPosition(m_window);
        bool h=getButtonRect(60,y,680,30).contains((float)m.x,(float)m.y);
        drawButton(ss.str(),60,y,680,30,h);
        y+=36;
    }
    sf::Vector2i m=sf::Mouse::getPosition(m_window);
    drawNarrationBox();
    drawButton("返回地图 (ESC)",300,430,200,40,getButtonRect(300,430,200,40).contains((float)m.x,(float)m.y));
    if(m_inputActive) renderInputBox();
}

void GUI::handleShopClick(int mx, int my) {
    if(getButtonRect(300,430,200,40).contains((float)mx,(float)my)){ m_state=GuiState::MapExploring; return; }
    float y=90;
    for(int id=1;id<=8;++id){
        if(getButtonRect(60,y,680,30).contains((float)mx,(float)my)){
            m_shopBuyId=id;
            auto& gm=GameManager::getInstance();
            auto& sh=gm.getShop();
            auto& ch=gm.getCharacter();
            if(sh.buyItem(id,ch)){ auto it=sh.findItem(id); if(it) gm.getInventory().addItem(it); }
            break;
        }
        y+=36;
    }
}

// ===== INVENTORY =====
void GUI::renderInventory() {
    sf::RectangleShape bg(sf::Vector2f(780,560)); bg.setPosition(10,10); bg.setFillColor(COLOR_CARD); bg.setOutlineColor(COLOR_OUTLINE); bg.setOutlineThickness(1); m_window.draw(bg);
    drawText("背包",400,30,24,COLOR_GOLD);
    auto& inv=GameManager::getInstance().getInventory();
    if(inv.itemCount()==0){
        drawText("这里空空如也",400,280,20,COLOR_GREY);
    } else {
        float y=80;
        auto items=inv.getAllItems();
        for(size_t i=0;i<items.size();++i){
            auto item=items[i];
            std::ostringstream ss1; ss1<<"ID:"<<item->getId()<<" "<<item->getName()<<" ["<<item->getType()<<"]";
            drawText(ss1.str(),400,y,14,COLOR_WHITE);
            drawText(item->getDescription(),400,y+20,12,COLOR_GREY);
            sf::Vector2i m=sf::Mouse::getPosition(m_window);
            drawButton("使用",650,y-6,80,28,getButtonRect(650,y-6,80,28).contains((float)m.x,(float)m.y));
            drawButton("丢弃",740,y-6,80,28,getButtonRect(740,y-6,80,28).contains((float)m.x,(float)m.y));
            y+=50;
        }
    }
    sf::Vector2i m=sf::Mouse::getPosition(m_window);
    drawNarrationBox();
    drawButton("返回地图 (ESC)",300,430,200,40,getButtonRect(300,430,200,40).contains((float)m.x,(float)m.y));
}

// ===== QUESTS =====
void GUI::renderQuests() {
    sf::RectangleShape bg(sf::Vector2f(780,560)); bg.setPosition(10,10); bg.setFillColor(COLOR_CARD); bg.setOutlineColor(COLOR_OUTLINE); bg.setOutlineThickness(1); m_window.draw(bg);
    drawText("任务列表",400,30,24,COLOR_GOLD);
    auto& qm=GameManager::getInstance().getQuestManager();
    float y=80;
    for(int id=1;id<=10;++id){
        auto* q=qm.mutableFindQuest(id);
        if(!q) continue;
        sf::Color c=COLOR_GREY;
        if(q->isCompleted()) c=COLOR_GREEN; else if(q->isAccepted()) c=COLOR_YELLOW;
        std::ostringstream ss; ss<<q->getName()<<" — "<<q->getDescription()<<" ("<<q->getCurrentCount()<<"/"<<q->getTargetCount()<<") 经验:"<<q->getRewardExp()<<" 金币:"<<q->getRewardGold();
        drawText(ss.str(),400,y,13,c);
        sf::Vector2i m=sf::Mouse::getPosition(m_window);
        if(!q->isAccepted()) drawButton("接受",620,y-6,60,24,getButtonRect(620,y-6,60,24).contains((float)m.x,(float)m.y));
        else if(!q->isCompleted()) drawButton("提交",700,y-6,60,24,getButtonRect(700,y-6,60,24).contains((float)m.x,(float)m.y));
        y+=38;
    }
    sf::Vector2i m=sf::Mouse::getPosition(m_window);
    drawNarrationBox();
    drawButton("返回地图 (ESC)",300,430,200,40,getButtonRect(300,430,200,40).contains((float)m.x,(float)m.y));
}

void GUI::handleInvClick(int mx, int my) {
    if(getButtonRect(300,430,200,40).contains((float)mx,(float)my)){ m_state=GuiState::MapExploring; return; }
    auto& inv=GameManager::getInstance().getInventory();
    float y=80;
    auto items=inv.getAllItems();
    for(size_t i=0;i<items.size();++i){
        auto item=items[i];
        if(getButtonRect(650,y-6,80,28).contains((float)mx,(float)my)){
            inv.useItem(item->getId(),GameManager::getInstance().getCharacter()); break;
        }
        if(getButtonRect(740,y-6,80,28).contains((float)mx,(float)my)){
            inv.removeItem(item->getId()); break;
        }
        y+=34;
    }
}

void GUI::handleQuestClick(int mx, int my) {
    if(getButtonRect(300,430,200,40).contains((float)mx,(float)my)){ m_state=GuiState::MapExploring; return; }
    auto& qm=GameManager::getInstance().getQuestManager();
    float y=80;
    for(int id=1;id<=10;++id){
        auto* q=qm.mutableFindQuest(id);
        if(!q){ y+=38; continue; }
        if(!q->isAccepted()&&getButtonRect(620,y-6,60,24).contains((float)mx,(float)my)) qm.acceptQuest(id);
        else if(q->isAccepted()&&!q->isCompleted()&&getButtonRect(700,y-6,60,24).contains((float)mx,(float)my)){ qm.completeQuest(id,GameManager::getInstance().getCharacter()); }
        y+=38;
    }
}

// ===== NARRATION BOX =====
void GUI::drawNarrationBox() {
    if(m_battleLog.empty()) return;
    sf::RectangleShape bg(sf::Vector2f(780,80));
    bg.setPosition(10,500);
    bg.setFillColor(sf::Color(0x1a,0x1a,0x30,220));
    bg.setOutlineColor(sf::Color(0x53,0x5b,0x7a));
    bg.setOutlineThickness(1);
    m_window.draw(bg);
    std::string text=m_battleLog.back();
    if(text.size()>60) text=text.substr(0,60)+"...";
    drawText(text,400,540,14,COLOR_WHITE);
}

// ===== CHARACTER PANEL =====
void GUI::renderCharacterPanel() {
    sf::RectangleShape bg(sf::Vector2f(400,400)); bg.setPosition(200,50); bg.setFillColor(COLOR_CARD); bg.setOutlineColor(COLOR_GOLD); bg.setOutlineThickness(2); m_window.draw(bg);
    auto& ch=GameManager::getInstance().getCharacter();
    drawText(ch.getName()+" — 等级"+std::to_string(ch.getLevel()),400,80,26,COLOR_GOLD);
    drawText("生命: "+std::to_string(ch.getHp())+"/"+std::to_string(ch.getMaxHp()),400,130,18,COLOR_GREEN);
    drawText("经验: "+std::to_string(ch.getExp())+" (需"+std::to_string(ch.getLevel()*100)+")",400,160,16,COLOR_BLUE);
    drawText("攻击: "+std::to_string(ch.getAttack()),400,200,16,COLOR_RED);
    drawText("防御: "+std::to_string(ch.getDefense()),400,230,16,COLOR_BLUE);
    drawText("金币: "+std::to_string(ch.getGold()),400,260,16,COLOR_YELLOW);
    drawNarrationBox();
    sf::Vector2i m=sf::Mouse::getPosition(m_window);
    drawButton("返回地图 (ESC)",300,430,200,44,getButtonRect(300,430,200,44).contains((float)m.x,(float)m.y));
}

void GUI::handleCharPanelClick(int mx, int my) { if(getButtonRect(300,430,200,44).contains((float)mx,(float)my)) m_state=GuiState::MapExploring; }

// ===== MENU =====
void GUI::renderMenuPopup() {
    sf::RectangleShape bg(sf::Vector2f(250,200)); bg.setPosition(275,200); bg.setFillColor(COLOR_CARD); bg.setOutlineColor(COLOR_GOLD); bg.setOutlineThickness(2); m_window.draw(bg);
    drawText("菜单",400,215,20,COLOR_GOLD);
    sf::Vector2i m=sf::Mouse::getPosition(m_window);
    drawButton("保存存档",300,250,200,36,getButtonRect(300,250,200,36).contains((float)m.x,(float)m.y));
    drawButton("返回主菜单",300,300,200,36,getButtonRect(300,300,200,36).contains((float)m.x,(float)m.y));
    drawButton("取消",300,350,200,36,getButtonRect(300,350,200,36).contains((float)m.x,(float)m.y));
}

void GUI::handleMenuClick(int mx, int my) {
    if(getButtonRect(300,250,200,36).contains((float)mx,(float)my)){ m_saveMode=true; refreshSaveFiles(); m_state=GuiState::SaveManager; m_menuOpen=false; }
    else if(getButtonRect(300,300,200,36).contains((float)mx,(float)my)){ m_state=GuiState::MainMenu; m_menuOpen=false; GameManager::getInstance().setState(GameState::MainMenu); }
    else if(getButtonRect(300,350,200,36).contains((float)mx,(float)my)) m_menuOpen=false;
}

// ===== GAME OVER =====
void GUI::renderGameOver() {
    drawText("游戏结束",400,200,48,COLOR_RED);
    drawText("你被击败了...",400,270,20,COLOR_GREY);
    sf::Vector2i m=sf::Mouse::getPosition(m_window);
    drawButton("返回主菜单",300,350,200,44,getButtonRect(300,350,200,44).contains((float)m.x,(float)m.y));
}

void GUI::handleGameOverClick(int mx, int my) {
    if(getButtonRect(300,350,200,44).contains((float)mx,(float)my)){ m_state=GuiState::MainMenu; GameManager::getInstance().setState(GameState::MainMenu); }
}

// ===== INPUT =====
void GUI::renderInputBox() {
    sf::RectangleShape bg(sf::Vector2f(400,40)); bg.setPosition(200,540); bg.setFillColor(sf::Color(0,0,0,200)); bg.setOutlineColor(COLOR_GOLD); bg.setOutlineThickness(1); m_window.draw(bg);
    std::string disp=m_inputPrompt+": "+m_inputBuffer+"_";
    drawText(disp,400,546,16,COLOR_YELLOW);
}

void GUI::processInputConfirm(const std::string& input) {
    if(input.empty()) return;
    if(m_state==GuiState::MainMenu){
        if(m_inputPrompt.find("角色")!=std::string::npos) newGameGUI(input);
        else GameManager::getInstance().loadGame(input);
    } else if(m_state==GuiState::SaveManager){
        if(m_saveMode){
            bool success=GameManager::getInstance().saveGame(input);
            if(success){
                showNarration("存档已保存到: "+input);
                m_state=GuiState::MapExploring;
                m_menuOpen=false;
            } else {
                showNarration("保存失败，请重试");
            }
        } else {
            GameManager::getInstance().loadGame(input);
            m_state=GuiState::MapExploring;
        }
    }
}

// ===== DRAW HELPERS =====
void GUI::drawText(const std::string& text, float x, float y, unsigned size, sf::Color color) {
    sf::Text t;
    t.setFont(m_font);
    t.setString(sf::String::fromUtf8(text.begin(),text.end()));
    t.setCharacterSize(size);
    t.setFillColor(color);
    sf::FloatRect b=t.getLocalBounds();
    t.setOrigin(b.left+b.width/2.f, b.top+b.height/2.f);
    t.setPosition(x,y);
    m_window.draw(t);
}

void GUI::drawButton(const std::string& label, float x, float y, float w, float h, bool hover) {
    sf::RectangleShape rect(sf::Vector2f(w,h));
    rect.setPosition(x,y);
    rect.setFillColor(hover?COLOR_HIGHLIGHT:COLOR_CARD);
    rect.setOutlineColor(hover?COLOR_GOLD:COLOR_OUTLINE);
    rect.setOutlineThickness(hover?2:1);
    m_window.draw(rect);
    sf::Text t;
    t.setFont(m_font);
    t.setString(sf::String::fromUtf8(label.begin(),label.end()));
    t.setCharacterSize(std::min(14u,(unsigned)(h*0.45f)));
    t.setFillColor(hover?COLOR_GOLD:COLOR_WHITE);
    sf::FloatRect b=t.getLocalBounds();
    t.setOrigin(b.left+b.width/2.f, b.top+b.height/2.f);
    t.setPosition(x+w/2.f, y+h/2.f);
    m_window.draw(t);
}

sf::FloatRect GUI::getButtonRect(float x, float y, float w, float h) { return sf::FloatRect(x,y,w,h); }

void GUI::drawHPBar(float x, float y, float w, float h, int cur, int max, sf::Color fg) {
    sf::RectangleShape bg(sf::Vector2f(w,h)); bg.setPosition(x,y); bg.setFillColor(sf::Color(40,40,40)); m_window.draw(bg);
    if(max>0){
        sf::RectangleShape bar(sf::Vector2f(w*cur/max,h)); bar.setPosition(x,y); bar.setFillColor(fg); m_window.draw(bar);
    }
}

// ===== BUILDING INTERIOR =====
void GUI::renderBuildingInterior() {
    sf::RectangleShape bg(sf::Vector2f(800,600)); bg.setFillColor(sf::Color(0x1a,0x1a,0x30)); m_window.draw(bg);
    sf::RectangleShape beam(sf::Vector2f(800,3)); beam.setFillColor(sf::Color(0x6e,0x5e,0x4a,120));
    for(int i=0;i<8;i++){ beam.setPosition(0,40.f+i*70); m_window.draw(beam); }
    drawText("【"+m_buildingName+"】",400,35,30,COLOR_GOLD);
    sf::Vector2i m=sf::Mouse::getPosition(m_window);
    float by=110;
    if(m_buildingName=="教学楼"){
        drawText("教学楼内部 — 黑板上写满了公式，空气中弥漫着粉笔灰",400,80,13,COLOR_GREY);
        drawButton("📋 查看课堂表（任务）",200,by,400,44,getButtonRect(200,by,400,44).contains((float)m.x,(float)m.y)); by+=60;
        drawButton("💻 计算机机房",200,by,400,44,getButtonRect(200,by,400,44).contains((float)m.x,(float)m.y)); by+=60;
        drawButton("📝 交作业（获得EXP）",200,by,400,44,getButtonRect(200,by,400,44).contains((float)m.x,(float)m.y)); by+=60;
        drawButton("👤 查看角色状态",200,by,400,44,getButtonRect(200,by,400,44).contains((float)m.x,(float)m.y)); by+=60;
    } else if(m_buildingName=="食堂"){
        drawText("食堂内部 — 窗口前排着长队，空气中飘着饭菜香",400,80,13,COLOR_GREY);
        drawButton("🍚 点一份套餐（15金币，恢复HP）",200,by,400,44,getButtonRect(200,by,400,44).contains((float)m.x,(float)m.y)); by+=60;
        drawButton("🥤 买瓶饮料（5金币，小恢复HP）",200,by,400,44,getButtonRect(200,by,400,44).contains((float)m.x,(float)m.y)); by+=60;
        drawButton("💬 跟打饭阿姨聊天（任务）",200,by,400,44,getButtonRect(200,by,400,44).contains((float)m.x,(float)m.y)); by+=60;
        drawButton("👤 查看角色状态",200,by,400,44,getButtonRect(200,by,400,44).contains((float)m.x,(float)m.y)); by+=60;
    } else if(m_buildingName=="小卖部"){
        drawText("小卖部内部 — 货架上琳琅满目，老板正在看报纸",400,80,13,COLOR_GREY);
        drawButton("🛍️ 浏览商品（商店）",200,by,400,44,getButtonRect(200,by,400,44).contains((float)m.x,(float)m.y)); by+=60;
        drawButton("🎒 整理背包",200,by,400,44,getButtonRect(200,by,400,44).contains((float)m.x,(float)m.y)); by+=60;
        drawButton("📋 帮老板跑腿（任务）",200,by,400,44,getButtonRect(200,by,400,44).contains((float)m.x,(float)m.y)); by+=60;
        drawButton("👤 查看角色状态",200,by,400,44,getButtonRect(200,by,400,44).contains((float)m.x,(float)m.y)); by+=60;
    } else if(m_buildingName=="宿舍"){
        drawText("宿舍内部 — 四张床位整齐排列，室友正在打游戏",400,80,13,COLOR_GREY);
        drawButton("🛏️ 休息一下（恢复HP）",200,by,400,44,getButtonRect(200,by,400,44).contains((float)m.x,(float)m.y)); by+=60;
        drawButton("🎒 整理背包",200,by,400,44,getButtonRect(200,by,400,44).contains((float)m.x,(float)m.y)); by+=60;
        drawButton("⚔️ 跟室友切磋（战斗）",200,by,400,44,getButtonRect(200,by,400,44).contains((float)m.x,(float)m.y)); by+=60;
        drawButton("👤 查看角色状态",200,by,400,44,getButtonRect(200,by,400,44).contains((float)m.x,(float)m.y)); by+=60;
    } else if(m_buildingName=="图书馆"){
        drawText("图书馆阅览室 — 一排排书架延伸到尽头，安静得只能听到翻书声",400,80,13,COLOR_GREY);
        drawButton("📖 借阅书籍",200,by,400,44,getButtonRect(200,by,400,44).contains((float)m.x,(float)m.y)); by+=60;
        drawButton("🧠 自习（获得知识EXP）",200,by,400,44,getButtonRect(200,by,400,44).contains((float)m.x,(float)m.y)); by+=60;
        drawButton("📋 查阅资料（任务）",200,by,400,44,getButtonRect(200,by,400,44).contains((float)m.x,(float)m.y)); by+=60;
        drawButton("👤 查看角色状态",200,by,400,44,getButtonRect(200,by,400,44).contains((float)m.x,(float)m.y)); by+=60;
    }
    drawNarrationBox();
    drawButton("离开（ESC）",300,430,200,44,getButtonRect(300,430,200,44).contains((float)m.x,(float)m.y));
}

void GUI::handleBuildingClick(int mx, int my) {
    auto& gm=GameManager::getInstance();
    auto& ch=GameManager::getInstance().getCharacter();
    if(getButtonRect(300,430,200,44).contains((float)mx,(float)my)){ m_state=GuiState::MapExploring; return; }
    if(m_buildingName=="教学楼"){
        if(getButtonRect(200,110,400,44).contains((float)mx,(float)my)){ m_state=GuiState::Quests; m_battleLog.clear(); m_battleLog.push_back("查看课堂任务表..."); }
        if(getButtonRect(200,170,400,44).contains((float)mx,(float)my)){ m_state=GuiState::ClassroomLab; m_battleLog.clear(); m_battleLog.push_back("走进计算机机房..."); }
        if(getButtonRect(200,230,400,44).contains((float)mx,(float)my)){ 
            auto& inv=GameManager::getInstance().getInventory();
            bool hasHomework=false;
            int foundId=-1;
            auto items=inv.getAllItems();
            for(auto& item : items){
                if(item->getType()=="任务道具"){
                    hasHomework=true;
                    foundId=item->getId();
                    break;
                }
            }
            if(!hasHomework){
                for(auto& item : items){
                    if(item->getType()=="书籍"){
                        hasHomework=true;
                        foundId=item->getId();
                        break;
                    }
                }
            }
            if(hasHomework&&foundId!=-1){
                inv.removeItem(foundId);
                ch.gainExp(25);
                m_battleLog.clear(); m_battleLog.push_back("作业已提交！老师点头赞许，获得25EXP！");
            } else {
                m_battleLog.clear(); m_battleLog.push_back("没有作业可交！请先去图书馆借书或领取任务！");
            }
        }
        if(getButtonRect(200,290,400,44).contains((float)mx,(float)my)){ m_state=GuiState::CharacterPanel; m_battleLog.clear(); m_battleLog.push_back("查看角色状态..."); }
    } else if(m_buildingName=="食堂"){
        if(getButtonRect(200,110,400,44).contains((float)mx,(float)my)){ 
            if(ch.getGold()>=15){
                ch.gainGold(-15);
                ch.setHp(ch.getMaxHp()); 
                m_battleLog.clear(); m_battleLog.push_back("花15金币点了一份套餐，HP回满了！"); 
            } else {
                m_battleLog.clear(); m_battleLog.push_back("金币不足，套餐需要15金币！");
            }
        }
        if(getButtonRect(200,170,400,44).contains((float)mx,(float)my)){ 
            if(ch.getGold()>=5){
                ch.gainGold(-5);
                int heal=ch.getMaxHp()/3; 
                ch.setHp(std::min(ch.getMaxHp(),ch.getHp()+heal)); 
                m_battleLog.clear(); m_battleLog.push_back("花5金币买了一瓶饮料，恢复了"+std::to_string(heal)+"HP！"); 
            } else {
                m_battleLog.clear(); m_battleLog.push_back("金币不足，需要5金币！");
            }
        }
        if(getButtonRect(200,230,400,44).contains((float)mx,(float)my)) m_state=GuiState::Quests;
        if(getButtonRect(200,290,400,44).contains((float)mx,(float)my)) m_state=GuiState::CharacterPanel;
    } else if(m_buildingName=="小卖部"){
        if(getButtonRect(200,110,400,44).contains((float)mx,(float)my)){ m_state=GuiState::InShop; m_battleLog.clear(); m_battleLog.push_back("走进了小卖部..."); }
        if(getButtonRect(200,170,400,44).contains((float)mx,(float)my)){ m_state=GuiState::Inventory; m_battleLog.clear(); m_battleLog.push_back("打开背包整理物品..."); }
        if(getButtonRect(200,230,400,44).contains((float)mx,(float)my)){ m_state=GuiState::Quests; m_battleLog.clear(); m_battleLog.push_back("老板递给你一张跑腿任务单..."); }
        if(getButtonRect(200,290,400,44).contains((float)mx,(float)my)){ m_state=GuiState::CharacterPanel; m_battleLog.clear(); m_battleLog.push_back("查看角色状态..."); }
    } else if(m_buildingName=="宿舍"){
        if(getButtonRect(200,110,400,44).contains((float)mx,(float)my)){ ch.setHp(ch.getMaxHp()); m_battleLog.clear(); m_battleLog.push_back("躺在床上睡了一觉，神清气爽！HP恢复了！"); }
        if(getButtonRect(200,170,400,44).contains((float)mx,(float)my)){ m_state=GuiState::Inventory; m_battleLog.clear(); m_battleLog.push_back("打开背包整理物品..."); }
        if(getButtonRect(200,230,400,44).contains((float)mx,(float)my)){ m_battleLog.clear(); m_battleLog.push_back("室友向你发起挑战！战斗开始！"); startNewBattle(); }
        if(getButtonRect(200,290,400,44).contains((float)mx,(float)my)){ m_state=GuiState::CharacterPanel; m_battleLog.clear(); m_battleLog.push_back("查看角色状态..."); }
    } else if(m_buildingName=="图书馆"){
        if(getButtonRect(200,110,400,44).contains((float)mx,(float)my)){ m_state=GuiState::LibraryBorrow; m_battleLog.clear(); m_battleLog.push_back("走向借书处..."); }
        if(getButtonRect(200,170,400,44).contains((float)mx,(float)my)){ m_state=GuiState::LibraryStudy; m_battleLog.clear(); m_battleLog.push_back("找了个安静的座位坐下..."); }
        if(getButtonRect(200,230,400,44).contains((float)mx,(float)my)){ m_state=GuiState::Quests; m_battleLog.clear(); m_battleLog.push_back("在书架间寻找任务相关的资料..."); }
        if(getButtonRect(200,290,400,44).contains((float)mx,(float)my)){ m_state=GuiState::CharacterPanel; m_battleLog.clear(); m_battleLog.push_back("查看角色状态..."); }
    }
}

// ===== NARRATION / PIXEL HELPERS =====
void GUI::showNarration(const std::string& msg) {
    m_battleLog.clear(); m_battleLog.push_back(msg);
}

void GUI::drawNarrationBar() {
    if(m_battleLog.empty()) return;
    sf::RectangleShape bar(sf::Vector2f(800,28));
    bar.setFillColor(sf::Color(0,0,0,180));
    bar.setPosition(0,0);
    m_window.draw(bar);
    drawText(m_battleLog[0],400,14,13,COLOR_YELLOW);
}
void GUI::drawPixelPlayer(float cx, float cy) {
    float s=16.f;
    // body
    sf::CircleShape body(s); body.setFillColor(sf::Color(0x4a,0x90,0xd9));
    body.setOutlineColor(sf::Color::Black); body.setOutlineThickness(1);
    body.setOrigin(s,s); body.setPosition(cx,cy+2); m_window.draw(body);
    // head
    sf::CircleShape head(s*0.5f); head.setFillColor(sf::Color(0xff,0xe0,0xbd));
    head.setOutlineColor(sf::Color::Black); head.setOutlineThickness(1);
    head.setOrigin(s*0.5f,s*0.5f); head.setPosition(cx,cy-s*0.6f); m_window.draw(head);
    // eyes
    sf::CircleShape eye(2.5f); eye.setFillColor(sf::Color::Black);
    eye.setPosition(cx-4,cy-s*0.75f-1); m_window.draw(eye);
    eye.setPosition(cx+2,cy-s*0.75f-1); m_window.draw(eye);
}
void GUI::drawPixelEnemy(float cx, float cy, const std::string& type) {
    sf::Color c=sf::Color(0xe7,0x4c,0x3c);
    if(type=="slime") c=sf::Color(0x2e,0xcc,0x40);
    else if(type=="goblin") c=sf::Color(0xf3,0x9c,0x12);
    sf::CircleShape body(18); body.setFillColor(c);
    body.setOutlineColor(sf::Color::Black); body.setOutlineThickness(1);
    body.setOrigin(18,18); body.setPosition(cx,cy); m_window.draw(body);
    sf::CircleShape eye(3); eye.setFillColor(sf::Color::Black);
    eye.setPosition(cx-6,cy-6); m_window.draw(eye);
    eye.setPosition(cx+3,cy-6); m_window.draw(eye);
}

void GUI::refreshSaveFiles() {
    m_saveFiles.clear();
    std::string saveDir = GameManager::getInstance().getSaveManager().getSaveDir();
#ifdef HAS_FILESYSTEM
    if (!fs::exists(saveDir)) return;
    for (const auto& entry : fs::directory_iterator(saveDir)) {
        if (entry.is_regular_file()) {
            m_saveFiles.push_back(entry.path().filename().string());
        }
    }
#else
#ifdef _WIN32
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile((saveDir + "*").c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                m_saveFiles.push_back(findData.cFileName);
            }
        } while (FindNextFile(hFind, &findData));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(saveDir.c_str());
    if (dir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_REG) {
                m_saveFiles.push_back(entry->d_name);
            }
        }
        closedir(dir);
    }
#endif
#endif
}

void GUI::renderSaveManager() {
    sf::RectangleShape bg(sf::Vector2f(780,560)); bg.setPosition(10,10); bg.setFillColor(COLOR_CARD); bg.setOutlineColor(COLOR_OUTLINE); bg.setOutlineThickness(1); m_window.draw(bg);
    drawText(m_saveMode?"保存游戏":"读取存档",400,30,24,COLOR_GOLD);
    sf::Vector2i m=sf::Mouse::getPosition(m_window);
    float y=80;
    if(m_saveFiles.empty()){
        drawText("暂无存档文件",400,y,18,COLOR_GREY);
        y+=40;
    } else {
        for(size_t i=0;i<m_saveFiles.size();++i){
            drawButton(m_saveFiles[i],100,y,500,36,getButtonRect(100,y,500,36).contains((float)m.x,(float)m.y));
            drawButton(m_saveMode?"覆盖保存":"加载",610,y,80,36,getButtonRect(610,y,80,36).contains((float)m.x,(float)m.y));
            drawButton("删除",700,y,80,36,getButtonRect(700,y,80,36).contains((float)m.x,(float)m.y));
            y+=44;
        }
    }
    drawButton("新建存档",100,y,250,36,getButtonRect(100,y,250,36).contains((float)m.x,(float)m.y));
    drawButton("返回",530,y,250,36,getButtonRect(530,y,250,36).contains((float)m.x,(float)m.y));
    if(m_inputActive) renderInputBox();
}

void GUI::handleSaveManagerClick(int mx, int my) {
    float y=80;
    if(m_saveFiles.empty()){
        y+=40;
    } else {
        for(size_t i=0;i<m_saveFiles.size();++i){
            if(getButtonRect(610,y,80,36).contains((float)mx,(float)my)){
                if(m_saveMode){
                    GameManager::getInstance().saveGame(m_saveFiles[i]);
                    showNarration("存档已保存到: "+m_saveFiles[i]);
                } else {
                    GameManager::getInstance().loadGame(m_saveFiles[i]);
                    m_state=GuiState::MapExploring;
                }
                m_menuOpen=false;
                return;
            }
            if(getButtonRect(700,y,80,36).contains((float)mx,(float)my)){
                std::string saveDir = GameManager::getInstance().getSaveManager().getSaveDir();
#ifdef HAS_FILESYSTEM
                fs::remove(saveDir+m_saveFiles[i]);
#else
#ifdef _WIN32
                DeleteFile((saveDir+m_saveFiles[i]).c_str());
#else
                remove((saveDir+m_saveFiles[i]).c_str());
#endif
#endif
                refreshSaveFiles();
                return;
            }
            y+=44;
        }
    }
    if(getButtonRect(100,y,250,36).contains((float)mx,(float)my)){
        if(m_saveMode){
            m_inputActive=true;
            m_inputBuffer.clear();
            m_inputPrompt="输入存档名";
        } else {
            m_state=GuiState::MainMenu;
            m_inputActive=true;
            m_inputBuffer.clear();
            m_inputPrompt="请输入角色名称";
        }
        return;
    }
    if(getButtonRect(530,y,250,36).contains((float)mx,(float)my)){
        m_inputActive=false;
        if(m_saveMode){
            m_state=GuiState::MapExploring;
            m_menuOpen=false;
        } else {
            m_state=GuiState::MainMenu;
        }
    }
}

// ===== LIBRARY BORROW =====
void GUI::renderLibraryBorrow() {
    sf::RectangleShape bg(sf::Vector2f(780,560)); bg.setPosition(10,10); bg.setFillColor(COLOR_CARD); bg.setOutlineColor(COLOR_OUTLINE); bg.setOutlineThickness(1); m_window.draw(bg);
    drawText("📖 图书馆借阅",400,30,24,COLOR_GOLD);
    drawText("选择一本书借阅，书籍会放入背包",400,60,13,COLOR_GREY);
    sf::Vector2i m=sf::Mouse::getPosition(m_window);
    float y=100;
    struct Book{std::string name;std::string desc;int price;} books[]={
        {"《编程从入门到放弃》","学习编程的经典读物",15},{"《高等数学》","令人头秃但必须学",20},{"《英语四级词汇》","背了忘忘了背",10},
        {"《校园生存指南》","教你如何在校园里生存",25},{"《游戏开发实战》","教你做游戏",30},{"《心理学导论》","了解人心",18}
    };
    for(auto& b:books){
        std::ostringstream ss; ss<<b.name<<" — "<<b.desc<<" ("<<b.price<<"金币)";
        drawButton(ss.str(),60,y,680,36,getButtonRect(60,y,680,36).contains((float)m.x,(float)m.y));
        y+=42;
    }
    drawNarrationBox();
    drawButton("返回图书馆 (ESC)",300,430,200,40,getButtonRect(300,430,200,40).contains((float)m.x,(float)m.y));
}

void GUI::handleLibraryBorrowClick(int mx, int my) {
    if(getButtonRect(300,430,200,40).contains((float)mx,(float)my)){ m_state=GuiState::BuildingInterior; return; }
    auto& ch=GameManager::getInstance().getCharacter();
    auto& inv=GameManager::getInstance().getInventory();
    float y=100;
    struct BookInfo{std::string name;std::string desc;int price;int expBonus;} books[]={
        {"《编程从入门到放弃》","学习编程的经典读物",15,15},{"《高等数学》","令人头秃但必须学",20,25},
        {"《英语四级词汇》","背了忘忘了背",10,10},{"《校园生存指南》","教你如何在校园里生存",25,20},
        {"《游戏开发实战》","教你做游戏",30,30},{"《心理学导论》","了解人心",18,18}
    };
    for(int i=0;i<6;i++){
        auto& b=books[i];
        if(getButtonRect(60,y,680,36).contains((float)mx,(float)my)){
            if(ch.getGold()>=b.price){
                ch.gainGold(-b.price);
                inv.addItem(std::make_shared<Book>(100+i, b.name, b.price, b.desc, b.expBonus));
                m_battleLog.clear(); m_battleLog.push_back("借阅了《"+b.name+"》！放入背包！");
            } else {
                m_battleLog.clear(); m_battleLog.push_back("金币不足，无法借阅！");
            }
            break;
        }
        y+=42;
    }
}

// ===== LIBRARY STUDY =====
void GUI::renderLibraryStudy() {
    sf::RectangleShape bg(sf::Vector2f(780,560)); bg.setPosition(10,10); bg.setFillColor(COLOR_CARD); bg.setOutlineColor(COLOR_OUTLINE); bg.setOutlineThickness(1); m_window.draw(bg);
    drawText("🧠 图书馆自习",400,30,24,COLOR_GOLD);
    drawText("静下心来学习，可以获得知识EXP",400,60,13,COLOR_GREY);
    auto& ch=GameManager::getInstance().getCharacter();
    drawText("当前等级: "+std::to_string(ch.getLevel())+"  经验: "+std::to_string(ch.getExp()),400,120,16,COLOR_BLUE);
    sf::Vector2i m=sf::Mouse::getPosition(m_window);
    drawButton("📚 认真学习（获得30EXP）",200,200,400,50,getButtonRect(200,200,400,50).contains((float)m.x,(float)m.y));
    drawButton("💤 小憩一会儿（恢复20%HP）",200,280,400,50,getButtonRect(200,280,400,50).contains((float)m.x,(float)m.y));
    drawNarrationBox();
    drawButton("返回图书馆 (ESC)",300,430,200,40,getButtonRect(300,430,200,40).contains((float)m.x,(float)m.y));
    const char* quotes[]={"书中自有黄金屋...","知识就是力量！","今天也要加油鸭！","学习使我快乐","一分耕耘一分收获"};
    drawText(quotes[rand()%5],400,480,14,COLOR_GREY);
}

void GUI::handleLibraryStudyClick(int mx, int my) {
    auto& ch=GameManager::getInstance().getCharacter();
    if(getButtonRect(300,430,200,40).contains((float)mx,(float)my)){ m_state=GuiState::BuildingInterior; return; }
    if(getButtonRect(200,200,400,50).contains((float)mx,(float)my)){
        ch.gainExp(30);
        m_battleLog.clear(); m_battleLog.push_back("认真学习了一会儿，获得30EXP！");
    }
    if(getButtonRect(200,280,400,50).contains((float)mx,(float)my)){
        int heal=ch.getMaxHp()/5;
        ch.setHp(std::min(ch.getMaxHp(),ch.getHp()+heal));
        m_battleLog.clear(); m_battleLog.push_back("小憩了一会儿，恢复了"+std::to_string(heal)+"HP！");
    }
}

// ===== CLASSROOM LAB =====
void GUI::renderClassroomLab() {
    sf::RectangleShape bg(sf::Vector2f(780,560)); bg.setPosition(10,10); bg.setFillColor(COLOR_CARD); bg.setOutlineColor(COLOR_OUTLINE); bg.setOutlineThickness(1); m_window.draw(bg);
    drawText("💻 计算机机房",400,30,24,COLOR_GOLD);
    drawText("一排排电脑整齐排列，风扇嗡嗡作响",400,60,13,COLOR_GREY);
    sf::Vector2i m=sf::Mouse::getPosition(m_window);
    drawButton("🎮 玩游戏（消耗精力，可能遇到Bug）",200,120,400,50,getButtonRect(200,120,400,50).contains((float)m.x,(float)m.y));
    drawButton("💻 编程练习（获得技术EXP）",200,200,400,50,getButtonRect(200,200,400,50).contains((float)m.x,(float)m.y));
    drawButton("🌐 上网冲浪（休息一下）",200,280,400,50,getButtonRect(200,280,400,50).contains((float)m.x,(float)m.y));
    drawNarrationBox();
    drawButton("返回教学楼 (ESC)",300,430,200,40,getButtonRect(300,430,200,40).contains((float)m.x,(float)m.y));
}

void GUI::handleClassroomLabClick(int mx, int my) {
    auto& ch=GameManager::getInstance().getCharacter();
    if(getButtonRect(300,430,200,40).contains((float)mx,(float)my)){ m_state=GuiState::BuildingInterior; return; }
    if(getButtonRect(200,120,400,50).contains((float)mx,(float)my)){
        if(rand()%100<30){
            const char* bugs[]={"Runtime Error!","Segmentation Fault!","Blue Screen!","Stack Overflow!","Memory Leak!"};
            m_battleLog.clear(); m_battleLog.push_back("遇到了"+std::string(bugs[rand()%5])+"战斗开始！");
            startNewBattle();
        } else {
            const char* results[]={"玩游戏很开心，但什么也没得到...","通关了！但手速太快没拿到奖励","服务器维护中，白玩了","队友太坑，心情变差","游戏太好玩，沉迷了一会儿"};
            m_battleLog.clear(); m_battleLog.push_back(results[rand()%5]);
        }
    }
    if(getButtonRect(200,200,400,50).contains((float)mx,(float)my)){
        ch.gainExp(25);
        m_battleLog.clear(); m_battleLog.push_back("完成了编程练习，获得25EXP！");
    }
    if(getButtonRect(200,280,400,50).contains((float)mx,(float)my)){
        int heal=ch.getMaxHp()/4;
        ch.setHp(std::min(ch.getMaxHp(),ch.getHp()+heal));
        const char* feelings[]={
            "刷了会儿短视频，笑得肚子痛，心情愉悦！",
            "逛了逛社交网站，看到各种八卦，吃瓜吃得很满足~",
            "看了部搞笑视频合集，压力瞬间消失了！",
            "在论坛上灌水聊天，感觉自己又活过来了",
            "刷到了一条神评论，笑出猪叫声，HP恢复了！",
            "冲浪两小时，眼睛有点酸但心情超好！"
        };
        m_battleLog.clear(); m_battleLog.push_back(std::string(feelings[rand()%6])+" 恢复了"+std::to_string(heal)+"HP！");
    }
}

// ===== CLASSROOM SUBMIT =====
void GUI::renderClassroomSubmit() {
    sf::RectangleShape bg(sf::Vector2f(780,560)); bg.setPosition(10,10); bg.setFillColor(COLOR_CARD); bg.setOutlineColor(COLOR_OUTLINE); bg.setOutlineThickness(1); m_window.draw(bg);
    drawText("📝 交作业",400,30,24,COLOR_GOLD);
    drawText("老师正在批改作业...",400,60,13,COLOR_GREY);
    auto& ch=GameManager::getInstance().getCharacter();
    drawText("当前经验: "+std::to_string(ch.getExp()),400,120,16,COLOR_BLUE);
    sf::Vector2i m=sf::Mouse::getPosition(m_window);
    drawButton("提交作业",300,200,200,50,getButtonRect(300,200,200,50).contains((float)m.x,(float)m.y));
    drawNarrationBox();
    drawButton("返回教学楼 (ESC)",300,430,200,40,getButtonRect(300,430,200,40).contains((float)m.x,(float)m.y));
}

void GUI::handleClassroomSubmitClick(int mx, int my) {
    auto& ch=GameManager::getInstance().getCharacter();
    if(getButtonRect(300,430,200,40).contains((float)mx,(float)my)){ m_state=GuiState::BuildingInterior; return; }
    if(getButtonRect(300,200,200,50).contains((float)mx,(float)my)){
        int exp=20+rand()%10;
        ch.gainExp(exp);
        m_battleLog.clear(); m_battleLog.push_back("作业批改完成！获得"+std::to_string(exp)+"EXP！");
    }
}

#endif // HAS_SFML
