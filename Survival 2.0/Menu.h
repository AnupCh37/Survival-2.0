
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

enum class MenuState {
    INTRO_MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};

class Menu {
public:
    Menu();
    ~Menu();

    void Load();
    void Update(float deltaTime, sf::RenderWindow& window, MenuState& gameState);
    void Draw(sf::RenderWindow& window, MenuState gameState);

    void setGameOverStats(int survivedTime, int wavesSurvived, int enemiesKilled);

private:

    sf::Font font;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;


    sf::Text titleText;
    sf::Text playButtonText;
    sf::Text exitButtonText;
    sf::RectangleShape playButton;
    sf::RectangleShape exitButton;


    sf::Text gameOverText;
    sf::Text statsText;
    sf::Text mainMenuButtonText;
    sf::RectangleShape mainMenuButton;


    bool playButtonHovered;
    bool exitButtonHovered;
    bool mainMenuButtonHovered;
    bool mousePressed;

    int survivalTime;
    int wavesSurvived;
    int enemiesKilled;


    void setupIntroMenu();
    void setupGameOverMenu();
    void updateIntroMenu(sf::RenderWindow& window, MenuState& gameState);
    void updateGameOverMenu(sf::RenderWindow& window, MenuState& gameState);
    void drawIntroMenu(sf::RenderWindow& window);
    void drawGameOverMenu(sf::RenderWindow& window);
    bool isMouseOverButton(const sf::RectangleShape& button, sf::RenderWindow& window);
    void updateButtonAppearance(sf::RectangleShape& button, sf::Text& text, bool hovered);
};