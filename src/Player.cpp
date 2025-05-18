#include "Player.hpp"

#include <algorithm>

#include "Constants.hpp"
#include "Assets.hpp"
#include "TileManager.hpp"

#include <cmath>
#include <iostream>

using namespace Constants;

Player::Player(): sprite(Assets::getTexture("character")) {
    sprite.setOrigin({168.f / 2, 210.f / 2});
    sprite.setScale({0.2f, 0.2f});
    sprite.setPosition({WIDTH / 2.f, HEIGHT - 2.f * TILE_SIZE});
}

void Player::update(float deltaTime, const TileManager& tileManager, float& gamePosition) {
    sf::Vector2f movement (0.f, 0.f);

    applyGravity();

    if (movingLeft) movement.x -= MOVE_SPEED;
    if (movingRight) movement.x += MOVE_SPEED;

    if (movement.x < 0 && facingRight) {
        sprite.scale({ -1.f, 1.f });
        facingRight = false;
    } else if (movement.x > 0 && !facingRight) {
        sprite.scale({ -1.f, 1.f });
        facingRight = true;
    };

    movement.y += verticalSpeed;

    movement *= deltaTime * SPEED_MULTIPLIER;

    sf::Sprite playerDummy = sprite;
    playerDummy.move(movement);
    const bool collision = tileManager.checkCollision(playerDummy.getGlobalBounds());
    bool collisionX = false;
    bool collisionY = false;
    if (collision) {
        sf::Sprite dummyPlayerX = sprite;
        dummyPlayerX.move({
            movement.x, 0
        });
        if (tileManager.checkCollision(dummyPlayerX.getGlobalBounds())) collisionX = true;

        sf::Sprite dummyPlayerY = sprite;
        dummyPlayerY.move({
            0, movement.y
        });
        if (tileManager.checkCollision(dummyPlayerY.getGlobalBounds())) collisionY = true;
        // if (playerDummy.getPosition().y < 0) collisionY = true;
    }

    // std::cout << "Collision: " << collision << ", Collision X: " << collisionX << ", Collision Y: " << collisionY << std::endl;

    if (collision && !collisionX && !collisionY) {
        collisionX = true;
        collisionY = true;
    }

    if (collisionY) {
        //leci w dół
        verticalSpeed = 0;
        if (movement.y > 0) inAir = false;
    }

    gamePosition += !collisionX ? movement.x : 0;
    gamePosition = std::max(gamePosition, 0.f);

    sprite.move ({
        0.f, !collisionY ? movement.y : 0
    });
}

void Player::draw(sf::RenderWindow& window) const {
    window.draw(sprite);
}

void Player::moveLeft(const bool enable) {
    movingLeft = enable;
}

void Player::moveRight(const bool enable) {
    movingRight = enable;
}

void Player::jump() {
    if (!inAir) {
        verticalSpeed = -JUMP_FORCE;
        inAir = true;
    }
}

void Player::applyGravity() {
    verticalSpeed += GRAVITY;
}

void Player::stopVertical() {
    verticalSpeed = 0.f;
    inAir = false;
}

void Player::setPosition(const sf::Vector2f &position) {
    sprite.setPosition(position);
}

const sf::Sprite& Player::getSprite() const {
    return sprite;
}

sf::Vector2f Player::getPosition() const {
    return sprite.getPosition();
}

bool Player::isInAir() const {
    return inAir;
}

std::pair<int, int> Player::getPlayerPosition(float gamePosition) const {
    return std::pair<int, int>(
            round(gamePosition / TILE_SIZE) + 8 - 1,
            round((HEIGHT - getPosition().y) / TILE_SIZE)
    );
}

void Player::handleInput (sf::Event& event) {
    if (auto const e = event.getIf<sf::Event::KeyPressed>()) {
        if (e->code == sf::Keyboard::Key::A) movingLeft = true;
        if (e->code == sf::Keyboard::Key::D) movingRight = true;
        if (e->code == sf::Keyboard::Key::W) jump();
    }
    if (auto const e = event.getIf<sf::Event::KeyReleased>())
    {
        if (e->code == sf::Keyboard::Key::A) movingLeft = false;
        if (e->code == sf::Keyboard::Key::D) movingRight = false;
    }
}

void Player::reset () {
    sprite.setPosition({ WIDTH / 2.f, HEIGHT - 2.f * TILE_SIZE });
    movingLeft = false;
    movingRight = false;
    verticalSpeed = 0.f;
}