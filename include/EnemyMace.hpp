#pragma once

#include <SFML/Graphics.hpp>
#include <map>
#include <vector>

struct Enemy {
    float y;
    bool isFalling;
    sf::Sprite sprite;
};

class EnemyMace {
    public:
        EnemyMace();

        void add(int x);
        void update(float deltaTime);

        void draw(sf::RenderWindow &window, float const &gamePosition);

        void resetPositions();

        void checkEnemies(int const &playerX);

        bool checkEnemyCollision(sf::FloatRect const &playerBounds) const;

        bool isEnemy(int x) const;

        bool isEnemyPrev(int x) const;

        std::map<int, std::vector<Enemy>> getEnemies();

        void clearEnemies();

    private:
        sf::Sprite sprite;
        std::map<int, std::vector<Enemy>> enemies;
};