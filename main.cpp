//todo
// disable air jump when falls of the edge - actually its a feature
// add clouds - done
// add menu/game over screen - done
// make clouds kill player - done
// background moving slower than tiles - done
// clean duplications - done
// bfs crashes on jump over map - done
// bfs crashes when i need to move back - done
// add spikes to some tiles
// only top tiles should be grass
// check collision only with possible blocks

//algorymt
// może liczyć spadek do samego dołu i LastReachable ustawic na dole
// a jesli nie ma nic na dole to jedno w prawo
// potem dodac liczenie ile czy mozliwe bylo przeskoczyc w prawo
// chyba trzeba bedzie zmienic na allReachablePositions dla kazdego x
// i sprawdzac czy mozna sie dostac z jednego z nich
// jump height na razie nic za bardzo nie zmienia -> to do ogarniecia

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

struct EnemyMace {
    int y;
    Sprite sprite;
    bool isFalling = false;
};

auto main() -> int
{
    // srand(time(nullptr));
    srand(1);
    
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
    auto backgroundSprite = Sprite(backgroundTexture);
    backgroundSprite.setScale(
        {
            WIDTH / backgroundSprite.getLocalBounds().size.x,
            HEIGHT / backgroundSprite.getLocalBounds().size.y
        }
    );

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

    auto const enemyMaceTexture = Texture("assets/mace.png"); //128x128
    auto enemyMaceSprite = Sprite(enemyMaceTexture);
    enemyMaceSprite.setScale(
        {
            static_cast<float>(2 * TILE_SIZE) / 128.f,
            static_cast<float>(2 * TILE_SIZE) / 128.f
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

    auto const fogTexture = Texture("assets/fog.png"); // 1920x1080
    auto fogSprite = Sprite(fogTexture);
    fogSprite.setScale(
        {
            WIDTH / fogSprite.getLocalBounds().size.x,
            HEIGHT / fogSprite.getLocalBounds().size.y
        }
    );
    
    float gamePosition = 0.0f;

    float fogPosition = 0.0f;
    float fogSpeed = 3.00f;
 
    float moveSpeed = 6.0f;
    float gravity = 0.5f;
    bool facingRight = true;

    float jumpForce = 15.0f;
    float isInAir = false;
    int maxJumpHeight = 2;

    float verticalSpeed = 0.0f;

    float speedMultiplier = 50.0f;

    float enemySpeed = 5.0f;

    bool isMenu = true;
    bool disableFog = true;
    
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
    Time elapsed;
    //Skalowanie do wielkości okna, to sie robi automatycznie
    // View view (FloatRect({0.f, 0.f}, {WIDTH, HEIGHT}));
    // view.setViewport(FloatRect({0.f, 0.f}, {1.f, 1.f}));
    // window.setView(view);

    map<int, vector<Tile>> tiles;
    vector<tuple<int, int, int>> circles;
    map<int, vector<EnemyMace>> enemies;
    // bo jak wyskakiwalem z mapy to był przypał
    vector<tuple<int, int, int>> lastReachablePositions = {{50 , 3, 0}}; // 50, 3
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

       auto clearTilesX = [&](int x)
    {
        tiles.erase(x);
    };

    auto generateNextX = [&](int x)
    {
        cout << "Generating next x: " << x << endl;
        clearTilesX(x);

        int minLastY = 10;
        for (auto& [lastX, lastY, jumpHeight] : lastReachablePositions) {
            if (lastY < minLastY) {
                minLastY = lastY;
            }
        }

        if (enemies.find(x - 1) != enemies.end()) { //jesli poprzednia kolumna jest w mapie
            addTile(x, 11, grassTileSprite);
            return;
        }

        if (rand() % 20  == 0 && minLastY < 8) {
            enemies[x].push_back({0, enemyMaceSprite, false});
            clearTilesX(x + 1); //wyczysc następną kolumnę
            addTile(x, 11, grassTileSprite);
            return;
        };

        for (int i = 0; i < 10; i++) {
            int random = rand() % 2;
            if (random == 0) {
                addTile(x, i, grassTileSprite);
            }
        }
    };
    
    auto resetEnemiesPositions = [&]()
    {
        for (auto& [x, enemy] : enemies) {
            for (auto& e : enemy) {
                e.sprite.setPosition({static_cast<float>(x * TILE_SIZE - gamePosition), 0});
                e.isFalling = false;
            }
        }
    };

    map<int, map<int, vector<tuple<int, int>>>> paths; // path for each y for each x

    auto isTile = [&](int x, int y)
    {
        if (tiles.find(x) == tiles.end()) return false;
        for (const auto& tile : tiles[x]) {
            if (tile.y == y) {
                return true;
            }
        }
        return false;
    };

    auto isEnemy = [&](int x) {
        return enemies.find(x) != enemies.end() || enemies.find(x - 1) != enemies.end();
    };

    //https://www.geeksforgeeks.org/breadth-first-search-or-bfs-for-a-graph/
    auto canReach = [&](int reachX)
    {
        cout << "Can reach: " << reachX << " " << lastReachablePositions.size() << endl;
        vector<tuple<int, int, int>> newLastReachablePositions;

        cout << "New last reachable positions: " << newLastReachablePositions.size() << endl;

        for (auto& [playerX, playerY, jumpHeight] : lastReachablePositions) {
            
            // i
            // cout << "Checking for: "

            // cout << "Player position: " << playerX << ", " << playerY << endl;
            
            // Make visited array large enough and initialize with false
            int minX = min(reachX, playerX) - 5;
            int maxX = max(reachX, playerX) + 5;
            int arraySize = maxX - minX + 10;
            vector<vector<vector<bool>>> visited(arraySize, vector<vector<bool>>(15, vector<bool>(maxJumpHeight + 2, false)));
            
            queue<tuple<int, int, int, vector<tuple<int, int>>>> q;
            q.push({playerX, playerY, jumpHeight, {
                {playerX, playerY}
            }});
            visited[playerX - minX][playerY][0] = true;

            
            while (!q.empty()) {
                auto [x, y, jumpHeight, path] = q.front();
                q.pop();
                
                if (x == reachX) {
                    // cout << "Reached x: " << x << " y: " << y << endl;

                    // bool enemyOnTop = isEnemy(x);

                    // cout << "Enemy on top: " << enemyOnTop << endl;

                    // if (!enemyOnTop) {
                    //     if (y > 0 && !isTile(x, y - 1)) {
                    //         y--;
                    //         path.push_back({x, y});
                    //     }

                    //     if (y == 0) {
                    //         continue;
                    //     }
                    // }

                    if (playerX == 53 && playerY == 3) {
                        cout << "Found path for " << playerX << ", " << playerY << endl;
                        for (auto& [x, y] : path) {
                            cout << "(" << x << ", " << y << ") ";
                        }
                        cout << endl;
                    }

                    if (paths.find(playerX) == paths.end() || paths[playerX].find(playerY) == paths[playerX].end()) {
                        paths[playerX][playerY] = path;
                    };

                    bool found = false;
                    for (auto& [lx, ly, ljumpHeight] : newLastReachablePositions) {
                        if (lx == x && ly == y) {                         
                            // ljumpHeight = min(jumpHeight, ljumpHeight);
                            if (jumpHeight < ljumpHeight) {
                                ljumpHeight = jumpHeight;
                                for (auto& [cx, cy, cjumpHeight] : circles) {
                                    if (cx == x && cy == y) {
                                        cjumpHeight = jumpHeight;
                                    }
                                }
                            }
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        newLastReachablePositions.push_back({x, y, jumpHeight});
                        circles.push_back({x, y, jumpHeight});
                    }
                }

                // Check all adjacent positions including backwards
                vector<pair<int, int>> moves = {
                    {1, 0},   // right
                    {-1, 0},  // left
                    {0, 1},   // up
                    {0, -1},
                    // {1, 1},
                    // {-1, 1},
                };

                for (auto [dx, dy] : moves) {
                    int nx = x + dx;
                    int ny = y + dy;

                    int newJumpHeight = 
                        dy > 0 ? 
                        jumpHeight + 1 :  // jezeli skacze to +1
                        isTile(nx, ny - 1) ? 0 :  // jezeli ide w bok lub dol i jest tile to 0
                        dx != 0 ? 
                        isTile(x, y - 1) ? 0 : jumpHeight + 1 : //jezeli ide w bok to jezeli podemna jest tile to 0
                        jumpHeight
                    ; 
                    // Moze zmienic jumpHeight na float i dodawac 0.5 jezeli ide w bok bo skok to jest 2,5 a nie 2
                
                    // Check bounds
                    if (nx < minX || nx > reachX || ny <= 0 || ny >= 10) {
                        continue;
                    }
                    
                    if (newJumpHeight > maxJumpHeight) {
                        continue;
                    }
                    
                    // Skip if already visited
                    if (visited[nx - minX][ny][newJumpHeight]) {
                        continue;
                    }

                    // Check if position is blocked by a tile

                    // for (auto& tile : tiles[nx]) {
                    //     if (tile.y == ny) {
                    //         isTile = true;
                    //         break;
                    //     }
                    // }
                    
                    // If position is free, add to queue
                    if (!isTile(nx, ny) && (!isEnemy(nx) || ny < 8)) {
                        visited[nx - minX][ny][newJumpHeight] = true;
                        vector<tuple<int, int>> newPath = path;
                        newPath.push_back({nx, ny});
                        q.push({nx, ny, newJumpHeight, newPath});
                        if (playerX == 53 && playerY == 3) {
                            cout << "Added to queue: " << nx << ", " << ny << endl;
                        }
                    }
                }
            }
        }
        if (newLastReachablePositions.size() > 0) {
            lastReachablePositions = newLastReachablePositions;
            return true;
        }
        
        return false;
    };

    auto drawCircle = [&](int x, int y, int jumpHeight)
    {
        CircleShape circle(10);
        circle.setPosition({
            static_cast<float>(x * TILE_SIZE - gamePosition + TILE_SIZE / 2 - 10), 
            static_cast<float>(HEIGHT - (y + 1) * TILE_SIZE + TILE_SIZE / 2 - 10)
        });
        circle.setFillColor(Color::Red);

        Text text(font);
        text.setCharacterSize(15);
        text.setString(to_string(jumpHeight));
        text.setPosition({
            static_cast<float>(x * TILE_SIZE - gamePosition + TILE_SIZE / 2 - text.getGlobalBounds().size.x / 2), 
            static_cast<float>(HEIGHT - (y + 1) * TILE_SIZE + TILE_SIZE / 2 - text.getGlobalBounds().size.y / 2)
        });

        window.draw(circle);
        window.draw(text);
    };

    auto getAngle = [&](int x1, int y1, int x2, int y2)
    {
        float angle = atan2(y2 - y1, x2 - x1);
        return angle;
    };

    auto drawPath = [&](int x)
    {
        if (paths.find(x) == paths.end() || paths[x].size() == 0) {
            return;
        }

        auto [playerX, playerY] = getPlayerPosition();

        int minYDistance = 1000;
        vector<tuple<int, int>> path;

        // cout << "Paths: " << paths[x].size() << endl;
        // for (auto& [y, pathX] : paths[x]) {
        //     cout << "Y: " << y << " Path: " << pathX.size() << endl;
        //     for (auto& [x, y] : pathX) {
        //         cout << "(" << x << ", " << y << ") ";
        //     }
        //     cout << endl;
        // }

        //find path that is closest to player
        for (auto& [y, pathX] : paths[x]) {
            int yDistance = abs(y - playerY);
            if (yDistance < minYDistance) {
                minYDistance = yDistance;
                path = pathX;
            }
        }


        for (int i = 0; i < path.size() - 1; i++) {
            auto [x1, y1] = path[i];
            auto [x2, y2] = path[i + 1];

            float length = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));

            RectangleShape rectangle;
            rectangle.setSize({
                length * TILE_SIZE,
                2
            });
            rectangle.setPosition({
                static_cast<float> (x1 * TILE_SIZE - gamePosition + TILE_SIZE / 2), 
                static_cast<float> (HEIGHT - (y1 + 1) * TILE_SIZE + TILE_SIZE / 2)
            });
            rectangle.setFillColor(Color::Blue);

            float angle = getAngle(x1, y1, x2, y2);

            // if (y2 > y1 && x2 > x1) {
            //     angle = -45;
            // }

            // if (y2 > y1 && x2 < x1) {
            //     angle = 135;
            // }

            // if (y2 > y1) { //up
            //     angle = -90;
            // }
            // else if (y2 < y1) { //down
            //     angle = 90;
            // }

            // if (x2 > x1) { //right
            //     angle = 0;
            // }
            // else if (x2 < x1) { //left
            //     angle = 180;
            // }



            // cout << "Angle: " << angle << endl;
            rectangle.rotate(
                radians(
                    -angle
                )
            );

            window.draw(rectangle);
        }
    };

    auto drawCircles = [&]()
    {
        for (auto& [x, y, jumpHeight] : circles) {
            drawCircle(x, y, jumpHeight);
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
                    fogSpeed = 3.00f;
                    moving = {false, false, false, false};
                    playerSprite.setPosition({WIDTH / 2.f, HEIGHT - 2.f * TILE_SIZE});
                    resetTilesPositions();
                    resetEnemiesPositions();
                }
            }
        }
    };

 

    auto drawClouds = [&]()
    {
        RectangleShape fog({
            fogPosition - gamePosition,
            static_cast<float>(HEIGHT)
        });
        fog.setPosition(
            {0, 0}
        );
        fog.setFillColor(Color(255, 100, 100, 200));
        window.draw(fog);
    };

    auto checkEnemyCollision = [&]()
    {
        for (auto& [x, enemy] : enemies) {
            for (auto& e : enemy) {
                if (const std::optional intersection = playerSprite.getGlobalBounds().findIntersection(e.sprite.getGlobalBounds()))
                {
                    cout << "Enemy collision detected" << endl;
                    isGameOver = true;
                    isMenu = true;
                }
            }
        }
    };

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

    auto drawEnemies = [&]()
    {
        for (auto& [x, enemy] : enemies) {
            for (auto& e : enemy) {
                e.sprite.setPosition(
                    {
                        static_cast<float>(x * TILE_SIZE - gamePosition), 
                        e.sprite.getPosition().y
                    }
                );
                if (e.sprite.getPosition().y < HEIGHT) {
                    window.draw(e.sprite);
                }
            }
        }
    };

    auto checkEnemies = [&]()
    {
        auto [playerX, playerY] = getPlayerPosition();
        if (enemies.find(playerX) != enemies.end()) {
            cout << "Enemies found at " << playerX << endl;
            for (auto& e : enemies[playerX]) {
                e.isFalling = true;
            }
        }
    };

    auto moveEnemies = [&]()
    {
        for (auto& [x, enemy] : enemies) {
            for (auto& e : enemy) {
                if (e.isFalling) {
                    e.sprite.move({
                        0.f, enemySpeed * elapsed.asSeconds() * speedMultiplier
                    });
                }
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

        if (dummyPlayerSpriteY.getPosition().y > HEIGHT + 100) {
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
                if (dummyPlayerSpriteY.getPosition().y < 0) {
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

        checkEnemyCollision();

        if (gamePosition + WIDTH / 2.f < fogPosition) {
            isGameOver = true;
            isMenu = true;
        }

        gamePosition += !collisionX ? movement.x : 0;
        gamePosition = max(gamePosition, 0.0f);

        fogSpeed *= 1.0001f;

        if (!disableFog) {
            fogPosition += fogSpeed * elapsed.asSeconds() * speedMultiplier;
        }

        auto [playerX, playerY] = getPlayerPosition();

        positionText.setString("X: " + to_string(playerX) + " Y: " + to_string(playerY));

        drawBackground();
        drawTiles();
        drawClouds();
        drawEnemies();
        window.draw(playerSprite);
        window.draw(positionText);
        drawCircles();
        drawPath(playerX);
    };

    // canReach(15);

    while (window.isOpen())
    {
        window.clear(Color::White);

        if (isMenu) {
            drawMenu();
        }
        else {
            elapsed = clock.restart();
            drawGame();
            processFrontX();
            checkEnemies();
            moveEnemies();
        }

        window.display();
    }

    return 0;
}