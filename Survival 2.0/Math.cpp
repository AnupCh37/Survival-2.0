#define _USE_MATH_DEFINES 
#include "Math.h"
#include <cmath> 
#include <SFML/Graphics.hpp>
#include <iostream>


sf::Vector2f Math::NormalizeVector(sf::Vector2f vector)
{
    float magnitude = std::sqrt(vector.x * vector.x + vector.y * vector.y);
    if (magnitude != 0.0f)
        return sf::Vector2f(vector.x / magnitude, vector.y / magnitude);
    else
        return sf::Vector2f(0.0f, 0.0f);
}

bool Math::DidRectCollision(const sf::FloatRect& rect1, const sf::FloatRect& rect2)
{
    if (rect1.left + rect1.width > rect2.left &&
        rect2.left + rect2.width > rect1.left &&
        rect1.top + rect1.height > rect2.top &&
        rect2.top + rect2.height > rect1.top)
    {
        std::cout << "Collision detected!\n";
        return true;
    }

    return false;
}


sf::Vector2f Math::GetRectCenter(const sf::FloatRect& rect)
{
    return sf::Vector2f(rect.left + rect.width / 2.0f, rect.top + rect.height / 2.0f);
}