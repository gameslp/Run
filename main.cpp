//todo
// disable air jump when falls of the edge
// add clouds
// add menu/game over screen - done
// make clouds kill
// background moving slower than tiles - done / waiting for fix
// clean duplications - done
// bfs crashes on jump over map - done
// bfs crashes when i need to move back - done

#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <queue>
#include <set>
using namespace std;
using namespace sf;

struct Movement {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
};

struct Tile {
    int y;
    Sprite sprite;
};

struct Cloud {
    int x;
    int y;
    Sprite sprite;
};

auto main() -> int
{
    unsigned int const WIDTH = 960;
    unsigned int const HEIGHT = 640;
    unsigned int const TILE_SIZE = 64;

    auto window = RenderWindow(
        VideoMode({WIDTH, HEIGHT}),
        "Run!",
        Style::Default,
        State::Windowed,
        ContextSettings{.antiAliasingLevel = 8}
    );

    window.setFramerateLimit(60);

    auto const backgroundTexture = Texture("assets/Background.png");
    auto const backgroundSprite = Sprite(backgroundTexture);

    auto const playerTexture = Texture("assets/character.png"); // 168x210
    auto playerSprite = Sprite(playerTexture);
    
    playerSprite.setOrigin(
        {168.f / 2.f, 210.f / 2.f}
    );
    playerSprite.setPosition(
        {
            WIDTH / 2.f,
            HEIGHT - 2.f * TILE_SIZE
        }
    );  

    playerSprite.setScale(
        {0.2f, 0.2f}
    );

    auto const cloud1Texture = Texture("assets/cloud1.png"); //128x128
    auto const cloud2Texture = Texture("assets/cloud2.png"); //128x128

    auto cloud1Sprite = Sprite(cloud1Texture);
    auto cloud2Sprite = Sprite(cloud2Texture);

    cloud1Sprite.setOrigin(
        {
            128.f,
            0.f
        }
    );

    cloud2Sprite.setOrigin(
        {
            128.f,
            0.f
        }
    );

    auto const grassTexture = Texture("assets/grass.png"); // 128x128
    auto grassTileSprite = Sprite(grassTexture);
    grassTileSprite.setScale(
        {
            static_cast<float>(TILE_SIZE) / 128.f,
            static_cast<float>(TILE_SIZE) / 128.f
        }
    );

    auto const dirtTexture = Texture("assets/dirt.png"); // 128x128
    auto dirtTileSprite = Sprite(dirtTexture);
    dirtTileSprite.setScale(
        {
            static_cast<float>(TILE_SIZE) / 128.f,
            static_cast<float>(TILE_SIZE) / 128.f
        }
    );
    
    float gamePosition = 0.0f;

    float fogPosition = 0.0f;
    float fogSpeed = 1.00f;
 
    float moveSpeed = 6.0f;
    float gravity = 0.5f;
    bool facingRight = true;

    float jumpForce = 15.0f;
    float isInAir = false;

    float verticalSpeed = 0.0f;

    float speedMultiplier = 50.0f;

    bool isMenu = true;
    
    Font font("assets/oswald.ttf");
    Text menuText(font);
    menuText.setCharacterSize(32);
    menuText.setString("Press SPACE to start");
    menuText.setPosition(Vector2f(
        WIDTH / 2.f - menuText.getGlobalBounds().size.x / 2.f, 
        HEIGHT / 2.f - menuText.getGlobalBounds().size.y / 2.f
    ));
    menuText.setFillColor(Color::Black);

    bool isGameOver = false;
    
    Text gameOverText(font);
    gameOverText.setCharacterSize(32);
    gameOverText.setString("GG");
    gameOverText.setPosition(Vector2f(
        WIDTH / 2.f - gameOverText.getGlobalBounds().size.x / 2.f, 
        HEIGHT / 2.f - gameOverText.getGlobalBounds().size.y / 2.f
        - menuText.getGlobalBounds().size.y - 100
    ));
    gameOverText.setFillColor(Color::Black);

    Text positionText(font);
    positionText.setCharacterSize(32);
    positionText.setString("X: " + to_string(gamePosition / TILE_SIZE) + " Y: " + to_string(HEIGHT - playerSprite.getPosition().y / TILE_SIZE));
    positionText.setPosition(Vector2f(
        10,
        10
    ));
    positionText.setFillColor(Color::Black);

    Movement moving;
    Clock clock;

    //Skalowanie do wielkości okna, to sie robi automatycznie
    // View view (FloatRect({0.f, 0.f}, {WIDTH, HEIGHT}));
    // view.setViewport(FloatRect({0.f, 0.f}, {1.f, 1.f}));
    // window.setView(view);

    map<int, vector<Tile>> tiles;
    vector<Cloud> clouds;
    vector<pair<int, int>> circles;

    auto addTile = [&](int x, int y, Sprite sprite)
    {
        tiles[x].push_back({y, sprite});
    };

    ifstream file("level.txt");
    if (!file.is_open()) {
        cerr << "Could not open level file!" << endl;
        return -1;
    }

    string line;
    while (getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;
        
        int x, y;

        istringstream iss(line);
        if (iss >> x >> y) {
            addTile(x, y, grassTileSprite);
        }
    }
    file.close();


    auto addCloud = [&](int x, int y, Sprite sprite)
    {
        clouds.push_back({x, y, sprite});
    };



    auto drawTiles = [&]()
    {
        for (auto& [x, tiles] : tiles)
        {
            if (x * TILE_SIZE > gamePosition - TILE_SIZE && x * TILE_SIZE < gamePosition + WIDTH + TILE_SIZE) {
                for (auto& tile : tiles)
                {
                    tile.sprite.setPosition(
                    {
                        static_cast<float>(x * TILE_SIZE - gamePosition), 
                        static_cast<float>(HEIGHT - (tile.y + 1) * TILE_SIZE)
                    }
                    );
                    window.draw(tile.sprite);
                }
            }
        }
    };

    auto getPlayerPosition = [&]()
    {
        return pair<int, int>(
            round(gamePosition / TILE_SIZE) + 8 - 1, 
            round((HEIGHT - playerSprite.getPosition().y) / TILE_SIZE));
    };

    auto resetTilesPositions = [&]()
    {
        for (auto& [x, tiles] : tiles)
        {
            for (auto& tile : tiles) {
                tile.sprite.setPosition(
                    {
                        0,
                        0
                    }
                );
            }
        }
    };

    auto drawCircle = [&](int x, int y)
    {
        CircleShape circle(10);
        circle.setPosition({
            static_cast<float>(x * TILE_SIZE - gamePosition + TILE_SIZE / 2 - 10), 
            static_cast<float>(HEIGHT - (y + 1) * TILE_SIZE + TILE_SIZE / 2 - 10)
        });
        circle.setFillColor(Color::Red);
        window.draw(circle);
    };

    auto drawCircles = [&]()
    {
        for (auto& [x, y] : circles) {
            drawCircle(x, y);
        }
    };

    auto drawBackground = [&]()
    {
        Sprite backgroundSprite1 = backgroundSprite;
        Sprite backgroundSprite2 = backgroundSprite;

        float backgroundPosition = (-gamePosition / 5.f);  // Slower movement
        float adjustedPosition = backgroundPosition - floor(backgroundPosition / WIDTH) * WIDTH;

        backgroundSprite1.setPosition(Vector2f(
            adjustedPosition,
            0.f
        ));
        backgroundSprite2.setPosition(Vector2f(
            adjustedPosition - WIDTH,
            0.f
        ));

        window.draw(backgroundSprite1);
        window.draw(backgroundSprite2);
    };

    auto drawMenu = [&]()
    {
        window.draw(menuText);
        if (isGameOver) {
            window.draw(gameOverText);
        }
        while (auto const event = window.pollEvent())
        {
            if (event->is<Event::Closed>()){window.close();}
            if (auto const e = event->getIf<Event::KeyPressed>())
            {
                if (e->code == Keyboard::Key::Space) {
                    isMenu = false;
                    isGameOver = false;
                    gamePosition = 0.0f;
                    fogPosition = 0.0f;
                    verticalSpeed = 0.0f;
                    moving = {false, false, false, false};
                    playerSprite.setPosition({WIDTH / 2.f, HEIGHT - 2.f * TILE_SIZE});
                    resetTilesPositions();
                }
            }
        }
    };

    auto clearTilesX = [&](int x)
    {
        tiles.erase(x);
    };

    auto generateNextX = [&](int x)
    {
        cout << "Generating next x: " << x << endl;
        clearTilesX(x);
        for (int i = 0; i < 10; i++) {
            int random = rand() % 2;
            if (random == 0) {
                addTile(x, i, grassTileSprite);
            }
        }
    };

    // bo jak wyskakiwalem z mapy to był przypał
    pair<int, int> lastReachablePosition = {50, 3};

    auto canReach = [&](int reachX)
    {
        auto [playerX, playerY] = lastReachablePosition;
        cout << "Player position: " << playerX << ", " << playerY << endl;
        
        // Make visited array large enough and initialize with false
        int minX = min(reachX, playerX) - 5;  // Add buffer on both sides
        int maxX = max(reachX, playerX) + 5;
        int arraySize = maxX - minX + 10;
        vector<vector<bool>> visited(arraySize, vector<bool>(15, false));
        
        // Store parent of each node to reconstruct path
        map<pair<int, int>, pair<int, int>> parent;
        
        queue<pair<int, int>> q;
        q.push({playerX, playerY});
        visited[playerX - minX][playerY] = true;

        while (!q.empty()) {
            auto [x, y] = q.front();
            q.pop();
            
            if (x == reachX) {
                cout << "Reached x: " << x << " y: " << y << endl;
                circles.push_back({x, y});
                lastReachablePosition = {x, y};
                return true;
            }

            // Check all adjacent positions including backwards
            vector<pair<int, int>> moves = {
                {1, 0},   // right
                {-1, 0},  // left
                {0, 1},   // up
                {0, -1}   // down
            };

            for (auto [dx, dy] : moves) {
                int nx = x + dx;
                int ny = y + dy;

                // Check bounds
                if (nx < minX || nx > maxX || ny < 0 || ny >= 10) {
                    continue;
                }
                
                // Skip if already visited
                if (visited[nx - minX][ny]) {
                    continue;
                }

                // Check if position is blocked by a tile
                bool isTile = false;
                for (auto& tile : tiles[nx]) {
                    if (tile.y == ny) {
                        isTile = true;
                        break;
                    }
                }

                // If position is free, add to queue
                if (!isTile) {
                    visited[nx - minX][ny] = true;
                    parent[{nx, ny}] = {x, y};
                    q.push({nx, ny});
                    cout << "Added to queue: " << nx << ", " << ny << endl;
                }
            }
        }
        
        return false;
    };

    // auto drawConnection = [&](int x1, int y1, int x2, int y2)
    // {
        
    // }

    set<int> xToProcess;

    auto processFrontX = [&]()
    {
        if (!xToProcess.empty()) {
            cout << "Processing front x: " << *xToProcess.begin() << endl;
            int x = *xToProcess.begin();
            xToProcess.erase(x);
            generateNextX(x);
            while (!canReach(x)) {
                cout << "Failed to reach " << x << endl;
                generateNextX(x);
            }
        }
    };

    auto drawGame = [&]()
    {
        int lastX = tiles.rbegin()->first;
        if (gamePosition / TILE_SIZE > lastX - 10) {
            xToProcess.insert(lastX + 1);            
        }

        Vector2f movement{0.f, 0.f};
        Time elapsed = clock.restart();
        while (auto const event = window.pollEvent())
        {
            if (event->is<Event::Closed>()){window.close();}
            if (auto const e = event->getIf<Event::KeyPressed>())
            {
                if (e->code == Keyboard::Key::A) moving.left = true;
                if (e->code == Keyboard::Key::D) moving.right = true;
                if (e->code == Keyboard::Key::W && !isInAir)
                {
                    verticalSpeed = -jumpForce;
                    isInAir = true;
                }
            }
            if (auto const e = event->getIf<Event::KeyReleased>())
            {
                if (e->code == Keyboard::Key::A) moving.left = false;
                if (e->code == Keyboard::Key::D) moving.right = false;
            }
        }

        verticalSpeed += gravity;

        if (moving.left) movement.x -= moveSpeed;
        if (moving.right) movement.x += moveSpeed;

        movement.y += verticalSpeed;
        
        if (movement.x < 0 && facingRight)
        {
            playerSprite.scale({-1.f, 1.f});
            facingRight = false;
        }
        else if (movement.x > 0 && !facingRight) {
            playerSprite.scale({-1.f, 1.f});
            facingRight = true;
        }

        movement *= elapsed.asSeconds() * speedMultiplier;

        Sprite dummyPlayerSpriteX = playerSprite;

        dummyPlayerSpriteX.move(
            {
                movement.x,
                0
            }
        );

        Sprite dummyPlayerSpriteY = playerSprite;

        dummyPlayerSpriteY.move(
            {
                0,
                movement.y
            }
        );

        if (dummyPlayerSpriteY.getPosition().y > HEIGHT) {
            isGameOver = true;
            isMenu = true;
        }

        bool collisionX = false;
        bool collisionY = false;

        for (auto& [x, tiles] : tiles)
        {
            for (auto& tile : tiles) {
                if (const std::optional intersection = dummyPlayerSpriteX.getGlobalBounds().findIntersection(tile.sprite.getGlobalBounds()))
                {
                    // cout << "Collision detected X" << endl;
                    collisionX = true;
                }
                if (const std::optional intersection = dummyPlayerSpriteY.getGlobalBounds().findIntersection(tile.sprite.getGlobalBounds()))
                {
                    // cout << "Collision detected Y" << endl;
                    collisionY = true;
                }
                if (collisionX && collisionY) {
                    break;
                }
            }
        }        

        if (collisionY)
        {
            verticalSpeed = 0;
            if (movement.y > 0) // Leci W Dół
            {
                isInAir = false;
            }
        }

        playerSprite.move(
            {
                0,
                !collisionY ? movement.y : 0
            }
        );

        gamePosition += !collisionX ? movement.x : 0;
        gamePosition = max(gamePosition, 0.0f);

        fogPosition += fogSpeed * elapsed.asSeconds() * speedMultiplier;

        auto [playerX, playerY] = getPlayerPosition();

        positionText.setString("X: " + to_string(playerX) + " Y: " + to_string(playerY));

        drawBackground();
        drawTiles();
        // drawClouds();
        window.draw(playerSprite);
        window.draw(positionText);
        drawCircles();
    };

    while (window.isOpen())
    {
        window.clear(Color::White);

        if (isMenu) {
            drawMenu();
        }
        else {
            drawGame();
            processFrontX();
        }

        window.display();
    }

    return 0;
}