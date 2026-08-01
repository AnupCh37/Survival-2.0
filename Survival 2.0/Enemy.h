#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include "Collision.h"
#include "Sound.h"

class Player;

class Enemy
{
public:
    Enemy(float sp = 100.0f);
    ~Enemy();

    void Draw(sf::RenderWindow& window);
    void ChangeHealth(int hp);
    void Update(float deltaTime, const sf::Vector2f& playerPos, Player& player, const std::vector<int>& tiles);
    bool Load(const sf::Vector2f pos);

    Collision collision;
    sf::Sprite sprite;
    sf::FloatRect getGlobalBounds() const { return sprite.getGlobalBounds(); }
    sf::RectangleShape boundingRectangle;
    int getHealth() const { return health; }

    Sound* soundSystem;

private:
    sf::Texture idleTexture;
    sf::Texture walkTexture;

    int health;
    int maxHealth;
    sf::Vector2i size;
    float speed;

    float attackCooldown;
    sf::Clock attackClock;

    float animationTimer;
    int currentFrame;
    float frameTime;
    int totalFrames;
    bool wasMoving;

    sf::RectangleShape healthBarBackground;
    sf::RectangleShape healthBarForeground;
};