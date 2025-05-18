#include "Game.hpp"
#include "Assets.hpp"
#include <iostream>

int main() {
    srand(time(nullptr));
    try {
        Assets::load();
        Game game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
