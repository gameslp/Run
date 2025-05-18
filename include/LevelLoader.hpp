#pragma once

#include "TileManager.hpp"
#include <fstream>
#include <string>
#include <filesystem>

class LevelLoader {
    public:
        static bool loadLevel(const std::string& dirName, TileManager& tileManager, EnemyMace& enemyMace, const sf::Sprite& tileSprite) {
            std::ifstream tilesFile(dirName + "/tiles.txt");
            if (!tilesFile.is_open()) {
                throw std::runtime_error("Failed to load: " + dirName + "/tiles.txt");
            }

            std::ifstream enemiesFile(dirName + "/enemies.txt");
            if (!enemiesFile.is_open()) {
                throw std::runtime_error("Failed to load: " + dirName + "/enemies.txt");
            }

            tileManager.clearLevel();
            enemyMace.clearEnemies();

            std::string line;
            while (std::getline(tilesFile, line)) {
                if (line.empty() || line[0] == '#') continue;
                int x, y;
                std::istringstream iss(line);
                if (iss >> x >> y) {
                    tileManager.addTile(x, y, tileSprite);
                }
            }

            tilesFile.close();

            while (std::getline(enemiesFile, line)) {
                if (line.empty() || line[0] == '#') continue;
                int x;
                std::istringstream iss(line);
                if (iss >> x) {
                    enemyMace.add(x);
                }
            }

            enemiesFile.close();
            return true;
        }
        static std::vector<std::string> getLevelSaves () {
            std::vector<std::string> levelSaves;
            std::filesystem::path path = "./levels/";
            if (std::filesystem::exists(path)) {
                for (auto const& entry : std::filesystem::directory_iterator(path)) {
                    if (entry.is_directory()) {
                        levelSaves.push_back(entry.path().filename().string());
                    }
                }
            }
            return levelSaves;
        }
};