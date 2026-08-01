#pragma once
#include <SFML/Graphics.hpp>

class Pause
{
public:
    Pause();
    ~Pause();

    void Draw(sf::RenderWindow& window);
    void Load();
    void Update(float deltaTime, sf::RenderWindow& window);

    bool isPausedActive() const { return isPaused; }
    void setPaused(bool val) { isPaused = val; }

private:
    sf::Sprite pauseButtonSprite;
    sf::Texture pauseButtonTexture;

    sf::Text pauseText;
    sf::Font font;

    bool isPaused;
};
