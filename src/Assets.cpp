#include "Assets.hpp"

#include <iostream>

std::map<std::string, sf::Texture> Assets::textures;
std::map<std::string, sf::Font> Assets::fonts;

void Assets::load() {
    sf::Texture texture;
    if (!texture.loadFromFile("assets/character.png"))
        throw std::runtime_error("Failed to load character.png");
    textures["character"] = texture;

    if (!texture.loadFromFile("assets/grass.png"))
        throw std::runtime_error("Failed to load grass.png");
    textures["grass"] = texture;

    if (!texture.loadFromFile("assets/dirt.png"))
        throw std::runtime_error("Failed to load dirt.png");
    textures["dirt"] = texture;

    if (!texture.loadFromFile("assets/mace.png"))
        throw std::runtime_error("Failed to load mace.png");
    textures["mace"] = texture;

    if (!texture.loadFromFile("assets/Background.png"))
        throw std::runtime_error("Failed to load Background.png");
    textures["background"] = texture;

    if (!texture.loadFromFile("assets/fog.png"))
        throw std::runtime_error("Failed to load fog.png");
    textures["fog"] = texture;

    sf::Font font;
    if (!font.openFromFile("assets/oswald.ttf"))
        throw std::runtime_error("Failed to load oswald.ttf");
    fonts["oswald"] = font;
}

sf::Texture& Assets::getTexture(const std::string& name) {
    if (!textures.contains(name)) {
        std::cerr << "Texture not found: " << name << std::endl;
    }
    return textures.at(name);
}

sf::Font& Assets::getFont(const std::string& name) {
    if (!fonts.contains(name)) {
        std::cerr << "Font not found: " << name << std::endl;
    }
    return fonts.at(name);
}
