#pragma once

#include <SFML/Graphics.hpp>

#include "TileManager.hpp"

class TileManager;

class Player {
    public:
        Player();

        void update (float deltaTime, const TileManager& tileManager, float& gamePosition);
        void draw (sf::RenderWindow& window) const;

        void moveLeft(bool enable);
        void moveRight(bool enable);
        void jump();

        void applyGravity();
        void stopVertical();
        void setPosition(const sf::Vector2f& position);

        const sf::Sprite& getSprite() const;
        sf::Vector2f getPosition() const;

        std::pair<int, int> getPlayerPosition(float gamePosition) const;

        void handleInput(sf::Event &event);

        void reset();

        bool isInAir() const;

    private:
        sf::Sprite sprite;
        bool facingRight = true;

        float verticalSpeed = 0.f;
        bool inAir = false;

        bool movingLeft = false;
        bool movingRight = false;
};
