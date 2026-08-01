#include "Pause.h"
#include <iostream>

extern bool paused;

Pause::Pause() : isPaused(false)
{
    pauseText.setCharacterSize(30);
}

Pause::~Pause() {}

void Pause::Draw(sf::RenderWindow& window) {
    window.draw(pauseButtonSprite);
    window.draw(pauseText);
}

void Pause::Load() {

    if (!font.loadFromFile("Assets/Fonts/arial.ttf")) {
        std::cout << "Error loading font" << std::endl;
    }
    else {
        std::cout << "Pause text loaded successfully" << std::endl;
    }

    pauseText.setFont(font);
    pauseText.setCharacterSize(20);
    pauseText.setFillColor(sf::Color::White);
    pauseText.setPosition(20.f, 70.f);

    if (!pauseButtonTexture.loadFromFile("Assets/UI/play.png")) {
        std::cout << "Error loading pause button texture!" << std::endl;
    }
    else {
        std::cout << "Pause texture loaded successfully." << std::endl;
    }

    pauseButtonTexture.setSmooth(true);
    pauseButtonSprite.setTexture(pauseButtonTexture);
    pauseButtonSprite.setTextureRect(sf::IntRect(0, 0, 9, 13));
    pauseButtonSprite.setColor(sf::Color(224, 224, 224));
    pauseButtonSprite.setPosition(10.f, 10.f);
    pauseButtonSprite.setScale(1.5f, 1.5f);
}

void Pause::Update(float deltaTime, sf::RenderWindow& window) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::FloatRect buttonBounds = pauseButtonSprite.getGlobalBounds();

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        if (buttonBounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
            if (!isPaused) {
                paused = !paused;
                isPaused = true;
                sf::sleep(sf::milliseconds(200));
            }
        }
    }
    else {
        isPaused = false;
    }
}
