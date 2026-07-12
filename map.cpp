#ifdef HAS_SFML

#include "map.h"

static const sf::Color C_GRASS(0x4a,0x8c,0x3f), C_ROAD(0xc4,0xb8,0x98);
static const sf::Color C_SCHOOL(0x5b,0x8c,0xb8), C_FOOD(0xe8,0x8d,0x3f);
static const sf::Color C_SHOP(0x8e,0x5e,0xb8), C_DORM(0x5e,0xb8,0x8e);
static const sf::Color C_LIB(0xb8,0x8e,0x5e), C_TREE(0x2a,0x5e,0x22), C_DOOR(0xff,0xe0,0x60);
static const sf::Color C_WATER(0x4a,0x8e,0xd4);

Map::Map() { m_grid.resize(m_height, std::vector<Tile>(m_width)); }
void Map::loadCampusMap() {
    for (int y=0;y<m_height;y++) for (int x=0;x<m_width;x++) { m_grid[y][x].type=0; m_grid[y][x].walkable=true; }
    for(int x=0;x<m_width;x++){m_grid[0][x].type=3;m_grid[0][x].walkable=false;m_grid[m_height-1][x].type=3;m_grid[m_height-1][x].walkable=false;}
    for(int y=0;y<m_height;y++){m_grid[y][0].type=3;m_grid[y][0].walkable=false;m_grid[y][m_width-1].type=3;m_grid[y][m_width-1].walkable=false;}
    auto r=[&](int x,int y){if(x>=0&&x<m_width&&y>=0&&y<m_height){m_grid[y][x].type=1;m_grid[y][x].walkable=true;}};
    for(int x=3;x<17;x++) r(x,4); for(int x=3;x<17;x++) r(x,9);
    for(int y=2;y<13;y++) r(5,y); for(int y=2;y<13;y++) r(10,y); for(int y=2;y<13;y++) r(15,y);
    for(int x=1;x<5;x++) r(x,2);
    // 连接路到各建筑门口
    for(int y=5;y<=8;y++) r(7,y);  // 教学楼竖路
    for(int y=5;y<=8;y++) r(13,y); // 食堂竖路
    for(int x=3;x<=5;x++) r(x,8);  // 小卖部门口横路
    for(int x=11;x<=13;x++) r(x,8); // 宿舍门口横路
    for(int y=10;y<=13;y++) r(8,y); // 图书馆竖路
    struct B{int x,y;sf::Color c;std::string n;}; B blds[]={{6,1,C_SCHOOL,"教学楼"},{11,1,C_FOOD,"食堂"},{1,5,C_SHOP,"小卖部"},{11,5,C_DORM,"宿舍"},{6,10,C_LIB,"图书馆"}};
    for(auto&b:blds){m_buildings.push_back({b.x,b.y,4,4,b.n,b.c});for(int yy=b.y;yy<b.y+4;yy++)for(int xx=b.x;xx<b.x+4;xx++){m_grid[yy][xx].type=2;m_grid[yy][xx].walkable=false;m_grid[yy][xx].label=b.n;}
        // 门的位置：食堂在右边，图书馆在上面，其他在底部
        int dx,dy;
        if(b.n=="食堂"){ dx=b.x+3; dy=b.y+2; m_grid[dy][dx].type=5;m_grid[dy][dx].walkable=true;m_grid[dy][dx].label=b.n; }
        else if(b.n=="图书馆"){ dx=b.x+2; dy=b.y; m_grid[dy][dx].type=5;m_grid[dy][dx].walkable=true;m_grid[dy][dx].label=b.n; }
        else{ dx=b.x+2; dy=b.y+3; m_grid[dy][dx].type=5;m_grid[dy][dx].walkable=true;m_grid[dy][dx].label=b.n; }
        }
    // 校门口
    m_grid[2][2].type=5;m_grid[2][2].walkable=true;m_grid[2][2].label="校门口";
    m_grid[2][13].type=4;m_grid[2][13].walkable=false;m_grid[3][13].type=4;m_grid[3][13].walkable=false;
    m_playerStart=sf::Vector2i(3,2);
}

Tile& Map::getTile(int x,int y){return m_grid[y][x];}
const Tile& Map::getTile(int x,int y)const{return m_grid[y][x];}
bool Map::isWalkable(int x,int y)const{return x>=0&&y>=0&&x<m_width&&y<m_height&&m_grid[y][x].walkable;}

void Map::draw(sf::RenderWindow& w,sf::Font& f){
    int ts=m_tileSize; sf::RectangleShape rect(sf::Vector2f((float)ts,(float)ts));
    for(int y=0;y<m_height;y++)for(int x=0;x<m_width;x++){int t=m_grid[y][x].type;rect.setPosition((float)(x*ts),(float)(y*ts));
        switch(t){case 0:rect.setFillColor(C_GRASS);break;case 1:rect.setFillColor(C_ROAD);break;case 3:rect.setFillColor(C_TREE);break;case 4:rect.setFillColor(C_WATER);break;}
        if(t==2||t==5)continue;rect.setOutlineColor(sf::Color(0,0,0,20));w.draw(rect);}
    // pixel buildings + 内嵌门
    for(auto&b:m_buildings){sf::RectangleShape big(sf::Vector2f(b.w*ts,b.h*ts));big.setPosition(b.x*ts,b.y*ts);big.setFillColor(b.color);w.draw(big);
        sf::Text txt;txt.setFont(f);txt.setString(sf::String::fromUtf8(b.name.begin(),b.name.end()));txt.setCharacterSize(16);txt.setFillColor(sf::Color::White);
        auto tb=txt.getLocalBounds();txt.setOrigin(tb.width/2.f,tb.height/2.f);txt.setPosition((b.x+2)*ts,(b.y+1.5f)*ts);w.draw(txt);
        // 门的位置：食堂在右边，图书馆在上面，其他在底部
        sf::RectangleShape door;
        sf::CircleShape knob(3);knob.setFillColor(sf::Color(0x8a,0x7a,0x30));
        if(b.name=="食堂"){
            door.setSize(sf::Vector2f(ts*0.4f, ts*0.8f));
            door.setPosition((b.x+4.f)*ts-ts*0.5f, (b.y+2.f)*ts+ts*0.1f);
            knob.setPosition((b.x+4.f)*ts-ts*0.6f, (b.y+2.f)*ts+ts*0.5f);
        } else if(b.name=="图书馆"){
            door.setSize(sf::Vector2f(ts*0.8f, ts*0.4f));
            door.setPosition((b.x+2.f)*ts+ts*0.1f, (b.y+0.f)*ts);
            knob.setPosition((b.x+2.f)*ts+ts*0.5f, (b.y+0.f)*ts+3.f);
        } else {
            door.setSize(sf::Vector2f(ts*0.8f, ts*0.4f));
            door.setPosition((b.x+2.f)*ts+ts*0.1f, (b.y+4.f)*ts-ts*0.5f);
            knob.setPosition((b.x+2.f)*ts+ts*0.5f, (b.y+4.f)*ts-ts*0.55f);
        }
        door.setFillColor(C_DOOR);door.setOutlineColor(sf::Color(0x8a,0x7a,0x30));door.setOutlineThickness(2);
        w.draw(door);
        w.draw(knob);}
}
const std::vector<Map::Building>& Map::getBuildings()const{return m_buildings;}
#endif
