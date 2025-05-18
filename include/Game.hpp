#pragma once

#include <SFML/Graphics.hpp>
#include "TileManager.hpp"
#include "EnemyMace.hpp"
#include "PathFinding.hpp"
#include "Constants.hpp"

#include <set>

class Game {
    public:
        Game();
        void run();

        void handleMenuInput(sf::Event &event);

        void handleTextInput(sf::Event &event);

        void updateDebugText();

        void updateShowingSaves();

        void updateShowingSavesText();

        void updateTextInputText();

    private:
        void pollEvents();

        void processFrontX();

        void update(float deltaTime);
        void render();
        void drawMenu();

        void drawBackground();

        void drawFog();

        void reset();

        sf::RenderWindow window;
        sf::Font font;
        sf::Text menuText;
        sf::Text gameOverText;
        sf::Text positionText;
        sf::Text scoreText;
        sf::Text debugText;
        sf::Text showingSavesText;
        sf::Text textInputText;
        sf::Text saveText;
        sf::Text textInputLabel;

        TileManager tileManager;
        EnemyMace enemyMace;
        PathFinding pathFinding;
        Player player;

        sf::Sprite grassTile;
        sf::Sprite background;

        bool isGameOver = false;
        bool isMenu = true;
        bool debug = false;
        bool showingSaves = false;
        bool isTyping = false;

        std::vector<std::string> levelSaves;

        int saveSelected = 0;

        std::string input = "";

        float gamePosition = 0.f;
        float fogPosition = 0.f;
        float fogSpeed = 3.0f;

        int score = 0;

        sf::Clock clock;

        std::set<int> xToProcess;
};