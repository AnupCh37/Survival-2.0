#define _USE_MATH_DEFINES
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Player.h"
#include "Enemy.h"
#include "FrameRate.h"
#include <windows.h>
#include "TileMap.h"
#include "Pause.h"
#include <array>
#include "Music.h"
#include "Sound.h"
#include "WaveManager.h"
#include "Menu.h"
#include <cmath>
#include <algorithm>

bool paused{};

sf::View getLetterboxView(sf::View view, int windowWidth, int windowHeight)
{
    float windowRatio = windowWidth / static_cast<float>(windowHeight);
    float viewRatio = view.getSize().x / view.getSize().y;
    float sizeX = 1.0f;
    float sizeY = 1.0f;
    float posX = 0.0f;
    float posY = 0.0f;

    if (windowRatio >= viewRatio) {
        sizeX = viewRatio / windowRatio;
        posX = (1.0f - sizeX) / 2.0f;
    }
    else {
        sizeY = windowRatio / viewRatio;
        posY = (1.0f - sizeY) / 2.0f;
    }

    view.setViewport(sf::FloatRect(posX, posY, sizeX, sizeY));
    return view;
}

std::vector<int> loadCSVFile(const std::string& filename)
{
    std::vector<int> tileData;
    std::ifstream inputFile(filename);
    std::string line;

    if (!inputFile.is_open())
        return tileData;

    while (std::getline(inputFile, line)) {
        std::stringstream ss(line);
        std::string cell;

        while (std::getline(ss, cell, ',')) {
            tileData.push_back(std::stoi(cell));
        }
    }

    return tileData;
}

void updateCamera(sf::View& gameView,
    const sf::Vector2f& playerPosition,
    float mapWidthPixels,
    float mapHeightPixels)
{
    sf::Vector2f center = playerPosition;

    const float halfWidth = gameView.getSize().x * 0.5f;
    const float halfHeight = gameView.getSize().y * 0.5f;

    if (mapWidthPixels > gameView.getSize().x) {
        center.x = std::clamp(center.x, halfWidth, mapWidthPixels - halfWidth);
    }
    else {
        center.x = mapWidthPixels * 0.5f;
    }

    if (mapHeightPixels > gameView.getSize().y) {
        center.y = std::clamp(center.y, halfHeight, mapHeightPixels - halfHeight);
    }
    else {
        center.y = mapHeightPixels * 0.5f;
    }

    gameView.setCenter(center);
}

void resetGame(Player& player, WaveManager& waveManager, std::vector<Enemy>& enemies,
    Sound* soundSystem, sf::Clock& gameTimer)
{
    player = Player();
    player.Load();
    player.soundSystem = soundSystem;

    waveManager = WaveManager(soundSystem);
    enemies.clear();

    paused = false;
    gameTimer.restart();
}

int main()
{
    constexpr int windowWidth = 800;
    constexpr int windowHeight = 600;

    constexpr int tilew = 16;
    constexpr int tileh = 16;
    constexpr int mapw = 100;
    constexpr int maph = 75;

    constexpr float cameraWidth = 640.0f;
    constexpr float cameraHeight = 480.0f;

    const float mapWidthPixels = static_cast<float>(mapw * tilew);
    const float mapHeightPixels = static_cast<float>(maph * tileh);

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    sf::RenderWindow window(
        sf::VideoMode(windowWidth, windowHeight),
        "Wave Survival Game",
        sf::Style::Default,
        settings
    );
    window.setFramerateLimit(60);

    sf::View gameView(sf::FloatRect(0.0f, 0.0f, cameraWidth, cameraHeight));
    sf::View hudView(sf::FloatRect(0.0f, 0.0f,
        static_cast<float>(windowWidth), static_cast<float>(windowHeight)));

    gameView = getLetterboxView(gameView, windowWidth, windowHeight);
    hudView = getLetterboxView(hudView, windowWidth, windowHeight);

    std::cout << "Wave Survival Game Starting..." << std::endl;

    MenuState gameState = MenuState::INTRO_MENU;
    Menu gameMenu;

    FrameRate framRate;
    Player player;
    TileMap backgroundLayer;
    Pause pausegame;
    Music music;
    Sound soundEffects;
    WaveManager waveManager(&soundEffects);
    std::vector<Enemy> enemies;

    sf::Clock gameTimer;
    int enemiesKilledCount = 0;
    int lastObservedWave = waveManager.getCurrentWave();

    gameMenu.Load();

    player.soundSystem = &soundEffects;
    soundEffects.loadSound("arrowShoot", "Assets/Sounds/arrow_shoot.ogg");
    soundEffects.loadSound("playerWalk", "Assets/Sounds/footsteps.ogg");
    soundEffects.loadSound("playerHurt", "Assets/Sounds/player_hurt.ogg");
    soundEffects.loadSound("playerDeath", "Assets/Sounds/player_death.ogg");
    soundEffects.loadSound("axeHit", "Assets/Sounds/axe_hit.ogg");

    sf::Font font;
    if (!font.loadFromFile("Assets/Fonts/OldLondon.ttf")) {
        std::cout << "Error loading font!" << std::endl;
    }

    sf::Text waveText;
    waveText.setFont(font);
    waveText.setCharacterSize(50);
    waveText.setFillColor(sf::Color::White);
    waveText.setStyle(sf::Text::Bold);

    sf::RectangleShape textBackground;
    textBackground.setFillColor(sf::Color(0, 0, 0, 150));

    sf::Text queueHud;
    queueHud.setFont(font);
    queueHud.setCharacterSize(16);
    queueHud.setFillColor(sf::Color::White);
    queueHud.setPosition(10.0f, 10.0f);

    framRate.Load();
    player.Load();
    pausegame.Load();

    std::vector<int> collisionData = loadCSVFile("levels/l1.csv");

    const std::size_t expectedTileCount = static_cast<std::size_t>(mapw * maph);
    if (collisionData.size() < expectedTileCount) {
        std::cerr << "levels/l1.csv does not contain enough tiles. Expected "
                  << expectedTileCount << ", found " << collisionData.size() << std::endl;
        return -1;
    }

    if (!backgroundLayer.load(
        "Assets/World/Prison/tilesheet.png",
        { tilew, tileh },
        collisionData.data(),
        mapw,
        maph))
    {
        return -1;
    }

    updateCamera(gameView, player.psprite.getPosition(), mapWidthPixels, mapHeightPixels);

    if (music.load("Assets/GameMusic/gameloop.ogg"))
        music.play(true);
    else
        std::cout << "Music file cannot be loaded." << std::endl;

    sf::Clock clock;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                if (gameState == MenuState::PLAYING) {
                    paused = !paused;
                    gameState = paused ? MenuState::PAUSED : MenuState::PLAYING;
                }
                else if (gameState == MenuState::PAUSED) {
                    paused = false;
                    gameState = MenuState::PLAYING;
                }
            }

            if (event.type == sf::Event::Resized) {
                gameView = getLetterboxView(gameView, event.size.width, event.size.height);
                hudView = getLetterboxView(hudView, event.size.width, event.size.height);
            }
        }

        sf::Time deltaTimeTimer = clock.restart();
        float deltaTime = deltaTimeTimer.asMicroseconds() / 1000.0f;

        switch (gameState)
        {
        case MenuState::INTRO_MENU:
        case MenuState::GAME_OVER:
            window.setView(hudView);
            gameMenu.Update(deltaTime, window, gameState);

            if (gameState == MenuState::PLAYING) {
                resetGame(player, waveManager, enemies, &soundEffects, gameTimer);
                enemiesKilledCount = 0;
                lastObservedWave = waveManager.getCurrentWave();
                updateCamera(gameView, player.psprite.getPosition(), mapWidthPixels, mapHeightPixels);
            }
            break;

        case MenuState::PLAYING:
            if (!paused) {
                waveManager.Update(deltaTime, enemies);

                const int currentWave = waveManager.getCurrentWave();
                if (currentWave != lastObservedWave) {
                    if (currentWave >= 6) {
                        player.ChangeHealth(100 - player.getHealth());
                        std::cout << "[HEALTH RESET] Wave " << currentWave
                                  << " started. Player health restored to 100." << std::endl;
                    }
                    lastObservedWave = currentWave;
                }

                const int aliveEnemies = waveManager.getAliveEnemyCount(enemies);
                enemiesKilledCount = waveManager.getTotalEnemiesSpawned() - aliveEnemies;

                for (auto& enemy : enemies) {
                    enemy.Update(deltaTime, player.psprite.getPosition(), player, collisionData);
                }

                window.setView(gameView);
                player.Update(deltaTime, enemies, window, collisionData, waveManager.getPlayerSpeed());

                updateCamera(gameView, player.psprite.getPosition(), mapWidthPixels, mapHeightPixels);
                framRate.Update(deltaTime);

                if (player.getHealth() <= 0) {
                    gameState = MenuState::GAME_OVER;
                    const int survivalTime = static_cast<int>(gameTimer.getElapsedTime().asSeconds());
                    const int wavesSurvived = waveManager.getCurrentWave() - 1;
                    gameMenu.setGameOverStats(survivalTime, wavesSurvived, enemiesKilledCount);
                }
            }
            break;

        case MenuState::PAUSED:
            window.setView(hudView);
            pausegame.Update(deltaTime, window);
            break;
        }

        window.clear();

        if (gameState == MenuState::PLAYING || gameState == MenuState::PAUSED) {
            window.setView(gameView);
            window.draw(backgroundLayer);

            for (auto& enemy : enemies)
                enemy.Draw(window);

            player.Draw(deltaTime, window);

            window.setView(hudView);

            if (waveManager.isWaveLoading()) {
                const float timeRemaining = waveManager.getTimeRemaining();
                const int nextWave = waveManager.getCurrentWave();

                const std::string waveMessage =
                    "Wave " + std::to_string(nextWave + 1) +
                    " starting in " + std::to_string(static_cast<int>(std::ceil(timeRemaining))) +
                    " seconds...";

                waveText.setString(waveMessage);

                const sf::FloatRect textBounds = waveText.getLocalBounds();
                waveText.setPosition(
                    (windowWidth - textBounds.width) / 2.0f,
                    (windowHeight - textBounds.height) / 2.0f
                );

                textBackground.setSize(sf::Vector2f(textBounds.width + 20.0f, textBounds.height + 20.0f));
                textBackground.setPosition(
                    waveText.getPosition().x - 10.0f,
                    waveText.getPosition().y - 10.0f
                );

                window.draw(textBackground);
                window.draw(waveText);
            }

            {
                const int queued = static_cast<int>(waveManager.getQueuedEnemyCount());
                const int alive = waveManager.getAliveEnemyCount(enemies);
                const int current = waveManager.getCurrentWave();

                std::string stateStr;
                switch (waveManager.getWaveState()) {
                case WaveState::PreparingWave: stateStr = "Preparing"; break;
                case WaveState::SpawningEnemies: stateStr = "Spawning"; break;
                case WaveState::Fighting: stateStr = "Fighting"; break;
                case WaveState::WaitingForNextWave: stateStr = "Waiting"; break;
                case WaveState::Finished: stateStr = "Finished"; break;
                }

                std::ostringstream oss;
                oss << "Current Wave: " << current << "\n";
                oss << "Queued Enemies: " << queued << "\n";
                oss << "Alive Enemies: " << alive << "\n";
                oss << "Wave State: " << stateStr;

                queueHud.setString(oss.str());
                window.draw(queueHud);
            }

            framRate.Draw(window);

            if (gameState == MenuState::PAUSED)
                pausegame.Draw(window);
        }

        if (gameState == MenuState::INTRO_MENU || gameState == MenuState::GAME_OVER) {
            if (gameState == MenuState::GAME_OVER) {
                window.setView(gameView);
                window.draw(backgroundLayer);

                for (auto& enemy : enemies)
                    enemy.Draw(window);

                player.Draw(deltaTime, window);
            }

            window.setView(hudView);
            gameMenu.Draw(window, gameState);
        }

        window.display();
    }

    music.stop();
    return 0;
}