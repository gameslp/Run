#include "Game.hpp"

#include <cmath>
#include <iostream>

#include "Assets.hpp"
#include "LevelLoader.hpp"
#include "LevelSaver.hpp"

using namespace Constants;

Game::Game()
    : window(sf::RenderWindow(
        sf::VideoMode({WIDTH, HEIGHT}),
        "RUN!",
        sf::Style::Default,
        sf::State::Windowed,
        sf::ContextSettings{.antiAliasingLevel = 8}
    )),
    grassTile(Assets::getTexture("grass")),
    background(Assets::getTexture("background")),
    menuText(Assets::getFont("oswald")),
    gameOverText(Assets::getFont("oswald")),
    positionText(Assets::getFont("oswald")),
    scoreText(Assets::getFont("oswald")),
    debugText(Assets::getFont("oswald")),
    showingSavesText(Assets::getFont("oswald")),
    textInputText(Assets::getFont("oswald")),
    saveText(Assets::getFont("oswald")),
    textInputLabel(Assets::getFont("oswald")),
    pathFinding(tileManager, enemyMace) {

    window.setFramerateLimit(60);

    menuText.setCharacterSize(40);
    menuText.setString("Press SPACE to start");
    menuText.setFillColor(sf::Color::Black);
    menuText.setPosition({WIDTH / 2.f - menuText.getLocalBounds().size.x / 2, 100});

    gameOverText.setCharacterSize(32);
    gameOverText.setString("GG");
    gameOverText.setFillColor(sf::Color::Black);
    gameOverText.setPosition({WIDTH / 2.f - 30, 10});

    positionText.setCharacterSize(20);
    positionText.setFillColor(sf::Color::Black);
    positionText.setPosition({10, 10});

    scoreText.setCharacterSize(32);
    scoreText.setFillColor(sf::Color::White);

    debugText.setCharacterSize(20);
    debugText.setFillColor(sf::Color::Black);

    showingSavesText.setCharacterSize(26);
    showingSavesText.setFillColor(sf::Color::Black);

    textInputText.setCharacterSize(20);
    textInputText.setFillColor(sf::Color::Black);

    saveText.setCharacterSize(32);
    saveText.setFillColor(sf::Color::Black);
    saveText.setString("Press ENTER to save");
    saveText.setFillColor(sf::Color::Blue);
    saveText.setPosition({
        WIDTH / 2.f - saveText.getLocalBounds().size.x / 2,
        HEIGHT / 2.f - saveText.getLocalBounds().size.y / 2
    });

    textInputLabel.setCharacterSize(30);
    textInputLabel.setFillColor(sf::Color::Black);
    textInputLabel.setString("Input SAVE name:");
    textInputLabel.setPosition({
    WIDTH / 2.f - textInputLabel.getLocalBounds().size.x / 2,
    HEIGHT / 2.f - textInputLabel.getLocalBounds().size.y / 2 - 150
    });

    grassTile.setScale({
        static_cast<float>(TILE_SIZE) / 128.f,
        static_cast<float>(TILE_SIZE) / 128.f
    });

    background.setScale({
        WIDTH / background.getLocalBounds().size.x,
        HEIGHT / background.getLocalBounds().size.y
    });

    try {
        LevelLoader::loadLevel("assets/start", tileManager, enemyMace, grassTile);
    } catch (const std::exception& e) {
        std::cerr << "Level loading failed: " << e.what() << std::endl;
    }

    updateDebugText();
    updateShowingSaves();
}

void Game::run() {
    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        pollEvents();

        if (isMenu) {
            window.clear(sf::Color::White);
            drawBackground();
            drawMenu();
        } else {
            update(deltaTime);
            render();
        }

        window.display();
    }
}

void Game::handleMenuInput(sf::Event &event) {
    if (auto const e = event.getIf<sf::Event::KeyPressed>()) {
        if (e->code == sf::Keyboard::Key::Space) reset();
        if (e->code == sf::Keyboard::Key::H) {
            debug = !debug;
            updateDebugText();
        }
        if (e->code == sf::Keyboard::Key::L) {
            showingSaves = !showingSaves;
            updateShowingSaves();
        }
        if (e->code == sf::Keyboard::Key::Enter && showingSaves) {
            LevelLoader::loadLevel("levels/" + levelSaves[saveSelected], tileManager, enemyMace, grassTile);
            showingSaves = false;
        }
        if (e->code == sf::Keyboard::Key::Enter && !showingSaves) {
            if (isGameOver) {
                isTyping = true;
            }
        }
        if (e->code == sf::Keyboard::Key::Down) {
            saveSelected = (saveSelected + 1) % levelSaves.size();
        }
        if (e->code == sf::Keyboard::Key::Up) {
            saveSelected = (saveSelected - 1) % levelSaves.size();
        }
    }
}

void Game::handleTextInput(sf::Event &event) {
    if (auto const e = event.getIf<sf::Event::KeyPressed>()) {
        if (e->code == sf::Keyboard::Key::Escape) isTyping = false;
        if (e->code == sf::Keyboard::Key::Enter) {
            if (LevelSaver::saveLevel(input, tileManager, enemyMace)) {
                updateShowingSaves();
                isTyping = false;
            }

        }
        if (e->code == sf::Keyboard::Key::Backspace && !input.empty()) {
            input.pop_back();
            updateTextInputText();
        }
    }
    //https://en.sfml-dev.org/forums/index.php?topic=26927.0
    if (auto const e = event.getIf<sf::Event::TextEntered>()) {
        if ((e->unicode >= 'A' && e->unicode <= 'Z') || (e->unicode >= 'a' && e->unicode <= 'z')) {
            input += static_cast<char>(e->unicode);
            updateTextInputText();
        }
    }
}

void Game::updateDebugText() {
    debugText.setString("Debug mode: " + std::string(debug ? "ON" : "OFF") + '\n' + "Press H to change");
    debugText.setFillColor(debug ? sf::Color::Red : sf::Color::Black);
    debugText.setPosition({
        WIDTH / 2 - debugText.getLocalBounds().size.x / 2,
        HEIGHT - debugText.getLocalBounds().size.y
    });
}

void Game::updateShowingSaves() {
    levelSaves = LevelLoader::getLevelSaves();
    saveSelected = 0;
    showingSavesText.setString("Showing saves: " + std::string(showingSaves ? "ON" : "OFF") + '\n' + "Press L to change");
    showingSavesText.setPosition({
        WIDTH / 2 - showingSavesText.getLocalBounds().size.x / 2,
        HEIGHT / 2 - showingSavesText.getLocalBounds().size.y / 2 - 100
    });
};

void Game::updateTextInputText() {
    textInputText.setString(input);
    textInputText.setPosition({
        WIDTH / 2 - textInputText.getLocalBounds().size.x / 2,
        HEIGHT / 2 - textInputText.getLocalBounds().size.y / 2
    });
};

void Game::pollEvents() {
    while (std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) window.close();
        if (!isMenu) player.handleInput(*event);
        if (isMenu) isTyping ? handleTextInput(*event) : handleMenuInput(*event);
    }
}

void Game::processFrontX() {
    if (!xToProcess.empty()) {
        int x = *xToProcess.begin();
        tileManager.generateNextX(x, pathFinding, enemyMace, grassTile);
        if (pathFinding.canReach(x)) xToProcess.erase(x);
    }
}

void Game::update(float deltaTime) {
    processFrontX();
    player.update(deltaTime, tileManager, gamePosition);

    if (enemyMace.checkEnemyCollision(player.getSprite().getGlobalBounds())) {
        isGameOver = true;
        isMenu = true;
        return;
    }

    if (player.getPosition().y > HEIGHT + 100) {
        isGameOver = true;
        isMenu = true;
        return;
    }

    fogSpeed *= 1.0001f;
    fogPosition += fogSpeed * deltaTime * SPEED_MULTIPLIER;

    if (gamePosition + WIDTH / 2.f < fogPosition) {
        isGameOver = true;
        isMenu = true;
    }

    enemyMace.checkEnemies(player.getPlayerPosition(gamePosition).first);
    enemyMace.update(deltaTime);

    int lastX = tileManager.getTiles().rbegin()->first;
    if (gamePosition / TILE_SIZE > lastX - 12) {
        xToProcess.insert(lastX + 1);
    };

    score = std::max(static_cast<float>(score), gamePosition / TILE_SIZE * fogSpeed);

}

void Game::render() {
    window.clear(sf::Color::White);
    drawBackground();

    tileManager.draw(window, gamePosition);
    enemyMace.draw(window, gamePosition);
    window.draw(player.getSprite());

    drawFog();

    if (debug) {
        auto [playerX, playerY] = player.getPlayerPosition(gamePosition);
        positionText.setString("X: " + std::to_string(playerX) + ", Y: " + std::to_string(playerY));

        window.draw(positionText);
        pathFinding.drawDebug(window, gamePosition, player);
    }

    scoreText.setString("Score: " + std::to_string(score));
    scoreText.setPosition({
        WIDTH / 2 - scoreText.getLocalBounds().size.x / 2,
        10
    });

    window.draw(scoreText);
}

void Game::drawMenu() {
    if (isTyping) {
        sf::RectangleShape rect;
        rect.setSize({
            textInputText.getLocalBounds().size.x + 50,
            textInputText.getLocalBounds().size.y + 20
        });
        rect.setPosition({
            WIDTH / 2 - rect.getLocalBounds().size.x / 2,
            HEIGHT / 2 - rect.getLocalBounds().size.y / 2
        });
        window.draw(rect);
        window.draw(textInputText);
        window.draw(textInputLabel);
        return;
    }
    window.draw(menuText);
    if (isGameOver) {
        window.draw(gameOverText);
        if (!showingSaves) window.draw(saveText);
    }
    window.draw(debugText);
    window.draw(showingSavesText);
    if (showingSaves) {
        for (int i = 0; i < levelSaves.size(); i++) {
            sf::Text levelOption(Assets::getFont("oswald"));
            levelOption.setFillColor(saveSelected == i ? sf::Color::Blue : sf::Color::Black);
            levelOption.setString(std::to_string(i + 1) + ". " + levelSaves[i]);
            levelOption.setPosition({
                WIDTH / 2 - levelOption.getLocalBounds().size.x / 2,
                HEIGHT / 2 + (levelOption.getLocalBounds().size.y + 10) * i
            });
            window.draw(levelOption);
        }
    }
}

void Game::drawBackground() {
    sf::Sprite backgroundSprite1 = background;
    sf::Sprite backgroundSprite2 = background;

    float backgroundPosition = (-gamePosition / 5.f);
    float adjustedPosition = backgroundPosition - floor(backgroundPosition / WIDTH) * WIDTH;

    backgroundSprite1.setPosition({
        adjustedPosition,
        0.f
    });

    backgroundSprite2.setPosition({
        adjustedPosition - WIDTH,
        0.f
    });

    window.draw(backgroundSprite1);
    window.draw(backgroundSprite2);
}

void Game::drawFog () {
    sf::RectangleShape fog({
    fogPosition - gamePosition,
    static_cast<float>(HEIGHT)
    });
    fog.setPosition({
        0,0
    });
    fog.setFillColor(sf::Color(255,100,100,200));
    window.draw(fog);
}

void Game::reset() {
    isMenu = false;
    isGameOver = false;
    gamePosition = 0.f;
    fogPosition = 0.f;
    fogSpeed = 3.0f;

    tileManager.resetTilePositions();
    enemyMace.resetPositions();
    player.reset();

    score = 0;
}
