cmake_minimum_required(VERSION 3.24)
project(gra2D VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)

FetchContent_MakeAvailable(SFML)

add_executable(gra2D
        src/TileManager.cpp
        src/Assets.cpp
        src/Player.cpp
        src/EnemyMace.cpp
        src/PathFinding.cpp
        src/Game.cpp
        main.cpp
        include/TileManager.hpp
        include/Constants.hpp
        include/Assets.hpp
        include/LevelLoader.hpp
        include/Player.hpp
        include/Game.hpp
        include/PathFinding.hpp
        include/EnemyMace.hpp
        include/LevelSaver.hpp
)

target_include_directories(gra2D PRIVATE include)
target_link_libraries(gra2D PRIVATE SFML::Graphics)
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR}/Debug)