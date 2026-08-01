#include "Menu.h"
#include <iostream>

Menu::Menu() :
    playButtonHovered(false),
    exitButtonHovered(false),
    mainMenuButtonHovered(false),
    mousePressed(false),
    survivalTime(0),
    wavesSurvived(0),
    enemiesKilled(0)
{
}

Menu::~Menu() {
}

void Menu::Load() {

    if (!font.loadFromFile("Assets/Fonts/OldLondon.ttf")) {
        if (!font.loadFromFile("Assets/Fonts/arial.ttf")) {
            std::cout << "Error: Could not load menu font!" << std::endl;
        }
    }

    setupIntroMenu();
    setupGameOverMenu();
}

void Menu::setupIntroMenu() {

    titleText.setFont(font);
    titleText.setString("WAVE SURVIVAL");
    titleText.setCharacterSize(64);
    titleText.setFillColor(sf::Color::White);
    titleText.setStyle(sf::Text::Bold);

    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
    titleText.setPosition(400.0f, 180.0f);


    playButton.setSize(sf::Vector2f(200.0f, 60.0f));
    playButton.setFillColor(sf::Color(50, 50, 50, 200));
    playButton.setOutlineThickness(3.0f);
    playButton.setOutlineColor(sf::Color::White);
    playButton.setOrigin(100.0f, 30.0f);
    playButton.setPosition(400.0f, 320.0f);

    playButtonText.setFont(font);
    playButtonText.setString("PLAY");
    playButtonText.setCharacterSize(32);
    playButtonText.setFillColor(sf::Color::White);
    sf::FloatRect playBounds = playButtonText.getLocalBounds();
    playButtonText.setOrigin(playBounds.width / 2.0f, playBounds.height / 2.0f);
    playButtonText.setPosition(400.0f, 320.0f);


    exitButton.setSize(sf::Vector2f(200.0f, 60.0f));
    exitButton.setFillColor(sf::Color(50, 50, 50, 200));
    exitButton.setOutlineThickness(3.0f);
    exitButton.setOutlineColor(sf::Color::White);
    exitButton.setOrigin(100.0f, 30.0f);
    exitButton.setPosition(400.0f, 420.0f);

    exitButtonText.setFont(font);
    exitButtonText.setString("EXIT");
    exitButtonText.setCharacterSize(32);
    exitButtonText.setFillColor(sf::Color::White);
    sf::FloatRect exitBounds = exitButtonText.getLocalBounds();
    exitButtonText.setOrigin(exitBounds.width / 2.0f, exitBounds.height / 2.0f);
    exitButtonText.setPosition(400.0f, 420.0f);
}

void Menu::setupGameOverMenu() {

    gameOverText.setFont(font);
    gameOverText.setString("GAME OVER");
    gameOverText.setCharacterSize(56);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setStyle(sf::Text::Bold);

    sf::FloatRect gameOverBounds = gameOverText.getLocalBounds();
    gameOverText.setOrigin(gameOverBounds.width / 2.0f, gameOverBounds.height / 2.0f);
    gameOverText.setPosition(400.0f, 150.0f);


    statsText.setFont(font);
    statsText.setCharacterSize(24);
    statsText.setFillColor(sf::Color::White);


    mainMenuButton.setSize(sf::Vector2f(250.0f, 60.0f));
    mainMenuButton.setFillColor(sf::Color(50, 50, 50, 200));
    mainMenuButton.setOutlineThickness(3.0f);
    mainMenuButton.setOutlineColor(sf::Color::White);
    mainMenuButton.setOrigin(125.0f, 30.0f);
    mainMenuButton.setPosition(400.0f, 450.0f);

    mainMenuButtonText.setFont(font);
    mainMenuButtonText.setString("MAIN MENU");
    mainMenuButtonText.setCharacterSize(28);
    mainMenuButtonText.setFillColor(sf::Color::White);
    sf::FloatRect mainMenuBounds = mainMenuButtonText.getLocalBounds();
    mainMenuButtonText.setOrigin(mainMenuBounds.width / 2.0f, mainMenuBounds.height / 2.0f);
    mainMenuButtonText.setPosition(400.0f, 450.0f);
}

void Menu::Update(float deltaTime, sf::RenderWindow& window, MenuState& gameState) {
    switch (gameState) {
    case MenuState::INTRO_MENU:
        updateIntroMenu(window, gameState);
        break;
    case MenuState::GAME_OVER:
        updateGameOverMenu(window, gameState);
        break;
    default:
        break;
    }
}

void Menu::updateIntroMenu(sf::RenderWindow& window, MenuState& gameState) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    bool currentMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);


    playButtonHovered = isMouseOverButton(playButton, window);
    updateButtonAppearance(playButton, playButtonText, playButtonHovered);

    if (playButtonHovered && currentMousePressed && !mousePressed) {
        gameState = MenuState::PLAYING;
    }


    exitButtonHovered = isMouseOverButton(exitButton, window);
    updateButtonAppearance(exitButton, exitButtonText, exitButtonHovered);

    if (exitButtonHovered && currentMousePressed && !mousePressed) {
        window.close();
    }

    mousePressed = currentMousePressed;
}

void Menu::updateGameOverMenu(sf::RenderWindow& window, MenuState& gameState) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    bool currentMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);


    std::string statsString = "Survival Time: " + std::to_string(survivalTime) + " seconds\n";
    statsString += "Waves Survived: " + std::to_string(wavesSurvived) + "\n";
    statsString += "Enemies Killed: " + std::to_string(enemiesKilled);
    statsText.setString(statsString);

    sf::FloatRect statsBounds = statsText.getLocalBounds();
    statsText.setOrigin(statsBounds.width / 2.0f, statsBounds.height / 2.0f);
    statsText.setPosition(400.0f, 280.0f);


    mainMenuButtonHovered = isMouseOverButton(mainMenuButton, window);
    updateButtonAppearance(mainMenuButton, mainMenuButtonText, mainMenuButtonHovered);

    if (mainMenuButtonHovered && currentMousePressed && !mousePressed) {
        gameState = MenuState::INTRO_MENU;
    }

    mousePressed = currentMousePressed;
}

void Menu::Draw(sf::RenderWindow& window, MenuState gameState) {
    switch (gameState) {
    case MenuState::INTRO_MENU:
        drawIntroMenu(window);
        break;
    case MenuState::GAME_OVER:
        drawGameOverMenu(window);
        break;
    default:
        break;
    }
}

void Menu::drawIntroMenu(sf::RenderWindow& window) {

    sf::RectangleShape overlay(sf::Vector2f(800.0f, 600.0f));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(overlay);


    window.draw(titleText);
    window.draw(playButton);
    window.draw(playButtonText);
    window.draw(exitButton);
    window.draw(exitButtonText);
}

void Menu::drawGameOverMenu(sf::RenderWindow& window) {

    sf::RectangleShape overlay(sf::Vector2f(800.0f, 600.0f));
    overlay.setFillColor(sf::Color(0, 0, 0, 200));
    window.draw(overlay);


    window.draw(gameOverText);
    window.draw(statsText);
    window.draw(mainMenuButton);
    window.draw(mainMenuButtonText);
}

bool Menu::isMouseOverButton(const sf::RectangleShape& button, sf::RenderWindow& window) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);
    return button.getGlobalBounds().contains(worldPos);
}

void Menu::updateButtonAppearance(sf::RectangleShape& button, sf::Text& text, bool hovered) {
    if (hovered) {
        button.setFillColor(sf::Color(100, 100, 100, 220));
        text.setFillColor(sf::Color::Yellow);
    }
    else {
        button.setFillColor(sf::Color(50, 50, 50, 200));
        text.setFillColor(sf::Color::White);
    }
}

void Menu::setGameOverStats(int survivedTime, int wavesSurvived, int enemiesKilled) {
    this->survivalTime = survivedTime;
    this->wavesSurvived = wavesSurvived;
    this->enemiesKilled = enemiesKilled;
}