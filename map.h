#pragma once

#ifdef HAS_SFML

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

struct BuildingDef {
    int x,y,w,h;
    std::string name;
    sf::Color color;
};

struct Tile {
    int type; bool walkable; std::string label;
};

class Map {
public:
    struct Building { int x,y,w,h; std::string name; sf::Color color; };

    Map();
    void loadCampusMap();
    void draw(sf::RenderWindow& window, sf::Font& font);
    Tile& getTile(int x, int y);
    const Tile& getTile(int x, int y) const;
    bool isWalkable(int x, int y) const;
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getTileSize() const { return m_tileSize; }
    sf::Vector2i getPlayerStart() const { return m_playerStart; }
    const std::vector<Building>& getBuildings() const;
private:
    std::vector<std::vector<Tile>> m_grid;
    std::vector<Building> m_buildings;
    int m_tileSize=40, m_width=20, m_height=15;
    sf::Vector2i m_playerStart;
};

#endif // HAS_SFML
