#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream> 
#include <cmath>    
#include "Collision.h"
#include "Sound.h"

class Enemy;

class Player
{
public:
    Player();
    ~Player();

    void Load();
    void Update(float deltaTime, std::vector<Enemy>& enemies, sf::RenderWindow& window, const std::vector<int>& tiles, float speed);
    void Draw(float deltaTime, sf::RenderWindow& window);
    void ChangeHealth(int hp);

    sf::Sprite psprite;
    sf::FloatRect getGlobalBounds() const { return psprite.getGlobalBounds(); }

    sf::RectangleShape boundingRectangle;
    int getHealth() const { return health; }


    Sound* soundSystem;

private:
    sf::Texture idleTexture;
    sf::Texture attackTexture;
    sf::Texture walkTexture;
    sf::Texture boneTexture;
    std::vector<sf::RectangleShape> boneBounds;
    sf::Vector2i size;

    float boneSpeed;
    float boneFireRate;
    sf::Clock boneFireClock;

    std::vector<sf::Sprite> bones;
    std::vector<sf::Vector2f> boneDirections;

    float animationTimer = 0.0f;
    int currentFrame = 0;
    float frameTime = 0.1f;
    int totalFrames = 6;

    int health;
    sf::Text healthText;
    sf::Font font;
    Collision collision;
    bool wasMoving = false;
};