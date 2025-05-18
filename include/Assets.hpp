#pragma once

#include <SFML/Graphics.hpp>
#include <map>
#include <string>

class Assets {
    public:
        static void load();

        static sf::Texture& getTexture(const std::string& name);
        static sf::Font& getFont(const std::string& name);

    private:
        static std::map<std::string, sf::Texture> textures;
        static std::map<std::string, sf::Font> fonts;
};