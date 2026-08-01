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

bool paused{};

sf::View getLetterboxView(sf::View view, int windowWidth, int windowHeight)
{
    float windowRatio = windowWidth / (float)windowHeight;
    float viewRatio = view.getSize().x / view.getSize().y;
    float sizeX = 1;
    float sizeY = 1;
    float posX = 0;
    float posY = 0;

    bool horizontalSpacing = true;
    if (windowRatio < viewRatio)
        horizontalSpacing = false;

    if (horizontalSpacing)
    {
        sizeX = viewRatio / windowRatio;
        posX = (1 - sizeX) / 2.f;
    }
    else
    {
        sizeY = windowRatio / viewRatio;
        posY = (1 - sizeY) / 2.f;
    }

    view.setViewport(sf::FloatRect(posX, posY, sizeX, sizeY));
    return view;
}

std::vector<int> loadCSVFile(const std::string& filename)
{
    std::vector<int> tileData;
    std::ifstream inputFile(filename);
    std::string line;

    if (!inputFile.is_open()) return tileData;

    while (std::getline(inputFile, line)) {
        std::stringstream ss(line);
        std::string cell;
        while (std::getline(ss, cell, ',')) {
            tileData.push_back(std::stoi(cell));
        }
    }
    inputFile.close();
    return tileData;
}


void resetGame(Player& player, WaveManager& waveManager, std::vector<Enemy>& enemies,
    Sound* soundSystem, sf::Clock& gameTimer) {
    // Reset player
    player = Player();
    player.Load();
    player.soundSystem = soundSystem;

    // Reset wave manager
    waveManager = WaveManager(soundSystem);

    // Clear and reset enemies
    enemies.clear();
    // WaveManager constructor prepares Wave 1; enemies remain empty until spawned over time

    paused = false;
    gameTimer.restart();
}

int main()
{
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode(800, 600), "Wave Survival Game", sf::Style::Default, settings);
    window.setFramerateLimit(60);
    sf::View view(sf::FloatRect(0, 0, 800, 600));
    window.setView(view);

    std::cout << "Wave Survival Game Starting..." << std::endl;

    MenuState gameState = MenuState::INTRO_MENU;
    Menu gameMenu;

    FrameRate framRate;
    Player player;
    TileMap backgroundLayer;
    TileMap foregroundLayer;
    Pause pausegame;
    Music music;
    Sound soundEffects;
    WaveManager waveManager(&soundEffects);
    std::vector<Enemy> enemies;

    sf::Clock gameTimer;
    int enemiesKilledCount = 0;

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

    // Queue HUD to display queue and wave info
    sf::Text queueHud;
    queueHud.setFont(font);
    queueHud.setCharacterSize(16);
    queueHud.setFillColor(sf::Color::White);
    queueHud.setPosition(10.0f, 10.0f);
    framRate.Load();
    player.Load();
    pausegame.Load();

    // WaveManager prepares Wave 1 internally; do not spawn immediately here.


    std::string backgroundFile = "levels/l1_background.csv";
    std::string foregroundFile = "levels/l1_foreground.csv";

    const int tilew = 16;
    const int tileh = 16;
    const int mapw = 50;
    const int maph = 38;

    std::vector<int> backgroundData = loadCSVFile(backgroundFile);
    std::vector<int> foregroundData = loadCSVFile(foregroundFile);

    if (backgroundData.empty())
        backgroundData = loadCSVFile("levels/l1.csv");


    if (music.load("Assets/GameMusic/gameloop.ogg"))
        music.play(true);
    else
        std::cout << "Music file cannot be loaded." << std::endl;


    if (!backgroundLayer.load("Assets/World/Prison/tilesheet.png", { tilew, tileh }, backgroundData.data(), mapw, maph))
        return -1;

    if (!foregroundData.empty()) {
        if (!foregroundLayer.load("Assets/World/Prison/tilesheet.png", { tilew, tileh }, foregroundData.data(), mapw, maph))
            return -1;
    }

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
                view = getLetterboxView(sf::View(sf::FloatRect(0, 0, 800, 600)), event.size.width, event.size.height);
                window.setView(view);
            }
        }

        sf::Time deltaTimeTimer = clock.restart();
        float deltaTime = deltaTimeTimer.asMicroseconds() / 1000.0f;


        switch (gameState) {
        case MenuState::INTRO_MENU:
        case MenuState::GAME_OVER:
            gameMenu.Update(deltaTime, window, gameState);


            if (gameState == MenuState::PLAYING) {
                resetGame(player, waveManager, enemies, &soundEffects, gameTimer);


                player.Load();

                enemiesKilledCount = 0;


                static int totalEnemiesSpawned = 0;
                static int currentWaveEnemies = 0;
                totalEnemiesSpawned = enemies.size();
                currentWaveEnemies = 1;
            }
            break;

        case MenuState::PLAYING:
            if (!paused) {

                // Pass delta time in milliseconds to WaveManager
                waveManager.Update(deltaTime, enemies);


                // Calculate alive enemies and kills using WaveManager's spawn counter
                int aliveEnemies = waveManager.getAliveEnemyCount(enemies);
                enemiesKilledCount = waveManager.getTotalEnemiesSpawned() - aliveEnemies;


                for (auto& enemy : enemies) {
                    enemy.Update(deltaTime, player.psprite.getPosition(), player, backgroundData);
                }

                player.Update(deltaTime, enemies, window, backgroundData, waveManager.getPlayerSpeed());
                framRate.Update(deltaTime);


                if (player.getHealth() <= 0) {
                    gameState = MenuState::GAME_OVER;
                    int survivalTime = static_cast<int>(gameTimer.getElapsedTime().asSeconds());
                    int wavesSurvived = waveManager.getCurrentWave() - 1;
                    gameMenu.setGameOverStats(survivalTime, wavesSurvived, enemiesKilledCount);
                }
            }
            break;

        case MenuState::PAUSED:
            pausegame.Update(deltaTime, window);
            break;
        }


        window.setView(view);
        window.clear();


        if (gameState == MenuState::PLAYING || gameState == MenuState::PAUSED) {
            window.draw(backgroundLayer);


            for (size_t i = 0; i < enemies.size(); i++) {
                enemies[i].Draw(window);
            }

            player.Draw(deltaTime, window);

            if (!foregroundData.empty())
                window.draw(foregroundLayer);

            if (waveManager.isWaveLoading()) {
                float timeRemaining = waveManager.getTimeRemaining();
                int nextWave = waveManager.getCurrentWave();

                std::string waveMessage = "Wave " + std::to_string(nextWave + 1) + " starting in " +
                    std::to_string((int)std::ceil(timeRemaining)) + " seconds...";

                waveText.setString(waveMessage);


                sf::FloatRect textBounds = waveText.getLocalBounds();
                waveText.setPosition(
                    (800 - textBounds.width) / 2.0f,
                    (600 - textBounds.height) / 2.0f
                );

                textBackground.setSize(sf::Vector2f(textBounds.width + 20, textBounds.height + 20));
                textBackground.setPosition(
                    waveText.getPosition().x - 10,
                    waveText.getPosition().y - 10
                );


                window.draw(textBackground);
                window.draw(waveText);
            }

            // Update and draw queue HUD showing FIFO queue state
            {
                int queued = static_cast<int>(waveManager.getQueuedEnemyCount());
                int alive = waveManager.getAliveEnemyCount(enemies);
                int current = waveManager.getCurrentWave();
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


            if (gameState == MenuState::PAUSED) {
                pausegame.Draw(window);
            }
        }

        if (gameState == MenuState::INTRO_MENU || gameState == MenuState::GAME_OVER) {

            if (gameState == MenuState::GAME_OVER) {
                window.draw(backgroundLayer);
                for (size_t i = 0; i < enemies.size(); i++) {
                    enemies[i].Draw(window);
                }
                player.Draw(deltaTime, window);
                if (!foregroundData.empty())
                    window.draw(foregroundLayer);
            }

            gameMenu.Draw(window, gameState);
        }

        window.display();
    }

    music.stop();
    return 0;
}