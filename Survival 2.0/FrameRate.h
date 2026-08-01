#pragma once
#include <SFML/Graphics.hpp>

class FrameRate {
public:
    FrameRate();
    ~FrameRate();

    void Initialize();
    void Load();
    void Update(float deltaTime);
    void Draw(sf::RenderWindow& window);

private:
    sf::Text text;
    sf::Font font;
    float timer;
    int frameCount;
};