#include "PathFinding.hpp"
#include "TileManager.hpp"
#include "EnemyMace.hpp"
#include "Constants.hpp"
#include "Player.hpp"
#include <queue>
#include <cmath>
#include <iostream>
#include <tuple>
#include <vector>

#include "Assets.hpp"

using namespace Constants;
using namespace std;

PathFinding::PathFinding(TileManager& tileManager, EnemyMace& enemyMace)
    : tileManager(tileManager), enemyMace(enemyMace) {
    lastReachablePositions = {{ 50, 3, 0 }};
}

//https://www.geeksforgeeks.org/breadth-first-search-or-bfs-for-a-graph/
bool PathFinding::canReach(const int reachX) {
    vector<tuple<int, int, int>> newLastReachable;

    for (auto& [playerX, playerY, jumpHeight] : lastReachablePositions) {
        const int minX = min(reachX, playerX) - 5;
        const int maxX = max(reachX, playerX) + 5;
        const int arraySize = maxX - minX + 10;

        vector<vector<vector<bool>>> visited(arraySize, vector<vector<bool>>(15, vector<bool>(MAX_JUMP_HEIGHT + 2, false)));

        queue<tuple<int, int, int, vector<tuple<int, int>>>> q;
        q.push({playerX, playerY, jumpHeight,
            {{playerX, playerY}}
        });


        visited[playerX - minX][playerY][0] = true;

        while (!q.empty()) {
            auto [x, y, jumpHeight, path] = q.front();
            q.pop();

            if (x == reachX) {
                if (!paths.contains(playerX) || !paths[playerX].contains(playerY)) {
                    paths[playerX][playerY] = path;
                }

                bool found = false;

                for (auto& [lx, ly, ljumpHeight] : newLastReachable) {
                    if (lx == x && ly == y) {
                        if (jumpHeight < ljumpHeight) {
                            ljumpHeight = jumpHeight;
                            for (auto& [cx, cy, cjumpHeight] : circles) {
                                cjumpHeight = jumpHeight;
                            }
                        }
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    newLastReachable.emplace_back(x, y, jumpHeight);
                    circles.emplace_back(x, y, jumpHeight);
                }
            }

            vector<pair<int, int>> moves = {
                {1, 0},
                {-1, 0},
                {0, 1},
                {0, -1},
                // {1, 1},
                // {-1, 1}
            };

            for (auto [dx, dy] : moves) {
                int nx = x + dx;
                int ny = y + dy;

                int newJumpHeight =
                    dy > 0 ?
                    jumpHeight + 1 :  // jezeli skacze to +1
                    tileManager.isTile(nx, ny - 1) ? 0 :  // jezeli ide w bok lub dol i jest tile to 0
                    dx != 0 ?
                    tileManager.isTile(x, y - 1) ? 0 : jumpHeight + 1 : //jezeli ide w bok to jezeli podemna jest tile to 0
                    jumpHeight
                ;

                if (nx < minX || nx > reachX || ny <= 0 || ny >= 10) {
                    continue;
                }

                if (newJumpHeight > MAX_JUMP_HEIGHT) {
                    continue;
                }

                if (visited[nx - minX][ny][newJumpHeight]) {
                    continue;
                }

                if (!tileManager.isTile(nx, ny) && (enemyMace.isEnemy(nx) || ny < 7)) {
                    visited[nx - minX][ny][newJumpHeight] = true;
                    vector<tuple<int, int>> newPath = path;
                    newPath.emplace_back( nx, ny );
                    q.emplace( nx, ny, newJumpHeight, newPath );
                }
            }
        }
    }

    if (!newLastReachable.empty()) {
            lastReachablePositions = newLastReachable;
            return true;
    }
    return false;
}

void PathFinding::drawDebug(sf::RenderWindow& window, float gamePosition, Player& player) {
    auto [playerX, playerY] = player.getPlayerPosition(gamePosition);
    for (const auto& [x, y, jump] : circles) {
        if (abs(playerX - x) > 6) continue;
        sf::CircleShape circle(10);
        circle.setFillColor(sf::Color::Red);
        circle.setPosition({
            static_cast<float>(x * TILE_SIZE - gamePosition + TILE_SIZE / 2 - 5),
            static_cast<float>(HEIGHT - (y + 1) * TILE_SIZE + TILE_SIZE / 2 - 10)
        });
        window.draw(circle);

        sf::Text text(Assets::getFont("oswald"), to_string(jump), 15);
        text.setPosition(circle.getPosition());
        window.draw(text);
    }
    drawPath(playerX, playerY, window, gamePosition);
}

auto getAngle(int x1, int y1, int x2, int y2) {
    double angle = atan2(y2 - y1, x2 - x1);
    return angle;
}

void PathFinding::drawPath(int playerX, int playerY, sf::RenderWindow& window, float gamePosition) {
    if (paths.find(playerX) == paths.end() || paths[playerX].size() == 0) return;

    int minYDistance = 1000;

    vector<tuple<int, int>> path;

    // std::cout << "Paths: " << paths[playerX].size() << endl;
    // for (auto& [y, pathX] : paths[playerX]) {
    //     std::cout << "Y: " << y << " Path: " << pathX.size() << endl;
    //     for (auto& [x, y] : pathX) {
    //         std::cout << "(" << x << ", " << y << ") ";
    //     }
    //     std::cout << endl;
    // }

    for (auto& [y, pathX] : paths[playerX]) {
        int yDistance = abs(y - playerY);
        if (yDistance < minYDistance) {
            minYDistance = yDistance;
            path = pathX;
        }
    }

    for (int i = 0; i < path.size() - 1; i++) {
        auto [x1, y1] = path[i];
        auto [x2, y2] = path[i + 1];

        const double length = sqrt(pow( x2 - x1, 2) + pow (y2 - y1, 2));

        sf::RectangleShape rectangle;
        rectangle.setSize({
            static_cast<float>(length * TILE_SIZE),
            2
        });
        rectangle.setPosition({
            static_cast<float> (x1 * TILE_SIZE - gamePosition + TILE_SIZE / 2),
            static_cast<float> (HEIGHT - (y1 + 1) * TILE_SIZE + TILE_SIZE / 2)
        });
        rectangle.setFillColor(sf::Color::Blue);

        const double angle = getAngle(x1, y1, x2, y2);

        rectangle.rotate(
            sf::radians(
                -angle
            )
        );

        window.draw(rectangle);
    }
}

std::vector<std::tuple<int, int, int>> PathFinding::getLastReachablePositions() const {
    return lastReachablePositions;
}

int PathFinding::lastReachableJump () {
    int minJumpHeight = 1000;
    for (auto& [playerX, playerY, jumpHeight] : lastReachablePositions) minJumpHeight = min(minJumpHeight, jumpHeight);
    return minJumpHeight;
}