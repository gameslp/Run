#pragma once

#include <SFML/Graphics.hpp>
#include <map>
#include <vector>

#include "PathFinding.hpp"
#include "EnemyMace.hpp"

class PathFinding;
class EnemyMace;

struct Tile {
    int y;
    sf::Sprite sprite;
};

class TileManager {
    public:
        TileManager ();

        void addTile (const int x, const int y, const sf::Sprite& sprite);
        void draw (sf::RenderWindow& window, float gamePosition);
        void resetTilePositions();
        void clearColumn (int x);
        bool isTile (int x, int y) const;
        const std::map<int, std::vector<Tile>>& getTiles() const;

        bool checkCollision(const sf::FloatRect& bounds) const;

        void generateNextX(int x, PathFinding& pf, EnemyMace& em, const sf::Sprite& tileSprite);

        void clearLevel();

    private:
        std::map<int, std::vector<Tile>> tiles;

        int lastEnemyX = 0;
};