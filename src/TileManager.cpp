#include "TileManager.hpp"
#include "Constants.hpp"
#include "PathFinding.hpp"
#include "EnemyMace.hpp"

using namespace Constants;

TileManager::TileManager () = default;

void TileManager::addTile (const int x, const int y, const sf::Sprite &sprite) {
    tiles[x].push_back({y, sprite});
}

void TileManager::draw(sf::RenderWindow& window, float gamePosition) {
    for (auto& [x, tiles] : tiles)
        {
            if (x * TILE_SIZE > gamePosition - TILE_SIZE && x * TILE_SIZE < gamePosition + WIDTH + TILE_SIZE) {
                for (auto&[y, sprite] : tiles)
                {
                    sprite.setPosition(
                    {
                        static_cast<float>(x * TILE_SIZE - gamePosition),
                        static_cast<float>(HEIGHT - (y + 1) * TILE_SIZE)
                    }
                    );
                    window.draw(sprite);
                }
            }
        }
}

void TileManager::resetTilePositions() {
    for (auto& [x, column] : tiles) {
        for (auto&[y, sprite] : column) {
            sprite.setPosition({0.f, 0.f});
        }
    }
}

void TileManager::clearColumn(const int x) {
    tiles.erase(x);
}

bool TileManager::isTile(const int x, const int y) const {
    const auto it = tiles.find(x);
    if (it == tiles.end()) return false;
    for (const auto& tile : it->second) {
        if (tile.y == y) return true;
    }
    return false;
}

const std::map<int, std::vector<Tile>>& TileManager::getTiles() const {
    return tiles;
}

bool TileManager::checkCollision(const sf::FloatRect& bounds) const {
    for (const auto& [x, column] : tiles) {
        for (const auto& tile : column) {
            if (const std::optional intersection = bounds.findIntersection(tile.sprite.getGlobalBounds())) {
                return true;
            }
        }
    }
    return false;
}

void TileManager::generateNextX (int x, PathFinding& pf, EnemyMace& em, sf::Sprite const& tileSprite) {
    clearColumn(x);

    int minLastY = 10;
    for (auto& [lastX, lastY, jumpHeight] : pf.getLastReachablePositions())
        minLastY = std::min(minLastY, jumpHeight);

    if (em.isEnemyPrev(x)) {
        addTile(x, 11, tileSprite);
        return;
    }

    if (rand() % 20 == 0 && minLastY < 7 && abs(x - lastEnemyX) > 3 && pf.lastReachableJump() == 0 ) {
        em.add(x);
        lastEnemyX = x;
        clearColumn(x + 1);
        addTile(x, 11, tileSprite);
        return;
    }

    for (int i = 0; i < 10; i++) if (rand()%2 == 0) addTile(x, i, tileSprite);
}

void TileManager::clearLevel() {
    tiles.clear();
    lastEnemyX = 0;
}
