#pragma once

#include "TileManager.hpp"
#include "EnemyMace.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>

class LevelSaver {
public:
    static bool saveLevel(const std::string& filename, TileManager& tileManager, EnemyMace& enemyMace) {
        auto tiles = tileManager.getTiles();
        auto enemies = enemyMace.getEnemies();
        std::filesystem::path path = "levels/" + filename;
        if (!std::filesystem::exists(path)) {
            if (!std::filesystem::create_directories(path)) {
               std::cerr << "Failed to create level directory: " << path << "\n";
               return false;
           }
        }

        std::fstream tilesOutput(path / "tiles.txt", std::ios::out | std::ios::trunc);
        if (tilesOutput.is_open()) {
            for (auto& [x, tiles] : tiles) {
                for (auto&[y, sprite] : tiles) tilesOutput << x << " " << y << "\n";
            }
            tilesOutput.close();
        } else {
            std::cerr << "Failed to open level file: " << path << "\n";
            return false;
        }

        std::fstream enemiesOutput(path / "enemies.txt", std::ios::out | std::ios::trunc);
        if (enemiesOutput.is_open()) {
            for (auto& [x, enemy] : enemies) {
                for (auto& e : enemy) enemiesOutput << x << "\n";
            }
            enemiesOutput.close();
        } else {
            std::cerr << "Failed to open level file: " << path << "\n";
            return false;
        }

        return true;
    }
};
