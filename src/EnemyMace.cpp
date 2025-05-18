#include "EnemyMace.hpp"
#include "TileManager.hpp"
#include "Constants.hpp"
#include "Player.hpp"
#include "Assets.hpp"

using namespace Constants;

EnemyMace::EnemyMace(): sprite(Assets::getTexture("mace")) {};

void EnemyMace::add (const int x) {
    enemies[x].push_back({0, false, sprite});
}

void EnemyMace::update (float deltaTime) {
    for (auto& [x, enemy] : enemies) {
        for (auto& e : enemy) {
            if (e.isFalling) e.y += ENEMY_SPEED * deltaTime * SPEED_MULTIPLIER;
        }
    }
}

void EnemyMace::draw(sf::RenderWindow& window, float const& gamePosition) {
    for (auto& [x, enemy] : enemies) {
        for (auto& e : enemy) {
            e.sprite.setPosition(
                {
                    static_cast<float>(x * TILE_SIZE - gamePosition),
                    e.y
                }
            );
            if (e.y < HEIGHT) {
                window.draw(e.sprite);
            }
        }
    }
}


void EnemyMace::resetPositions() {
    for (auto& [x, enemy] : enemies) {
        for (auto& e : enemy) {
            e.y = 0;
            e.isFalling = false;
        }
    }
}

void EnemyMace::checkEnemies (int const& playerX) {
    if (enemies.find(playerX) != enemies.end()) {
        for (auto& e : enemies[playerX]) {
            e.isFalling = true;
        }
    }
}

bool EnemyMace::checkEnemyCollision (sf::FloatRect const& playerBounds) const {
    for (auto& [x, enemy] : enemies) {
        for (auto& e : enemy) {
            if (const std::optional intersection = playerBounds.findIntersection(e.sprite.getGlobalBounds()))
            {
                return true;
            }
        }
    }
    return false;
}

bool EnemyMace::isEnemy (const int x) const {
    return enemies.find(x) != enemies.end() || enemies.find(x - 1) != enemies.end();
};

bool EnemyMace::isEnemyPrev (const int x) const {
    return enemies.find(x - 1) != enemies.end();
}

std::map<int, std::vector<Enemy>> EnemyMace::getEnemies() {
    return enemies;
}

void EnemyMace::clearEnemies() {
    enemies.clear();
}