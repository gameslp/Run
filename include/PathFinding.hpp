#pragma once

#include <SFML/Graphics.hpp>
#include <map>
#include <vector>
#include <tuple>

#include "TileManager.hpp"
#include "EnemyMace.hpp"
#include "Player.hpp"

class TileManager;
class Player;

class PathFinding {
    public:
        PathFinding(TileManager& tileManager, EnemyMace& enemyMace);

        bool canReach (int reachX);
        void drawDebug(sf::RenderWindow &window, float gamePosition, Player &player);

        std::vector<std::tuple<int, int, int>> getLastReachablePositions() const;

        int lastReachableJump();

    private:
        TileManager& tileManager;
        EnemyMace& enemyMace;

        std::vector<std::tuple<int, int, int>> lastReachablePositions;
        std::vector<std::tuple<int, int, int>> circles;

        std::map<int, std::map<int, std::vector<std::tuple<int, int>>>> paths;

        void drawPath(int x, int playerY, sf::RenderWindow &window, float gamePosition);
};
