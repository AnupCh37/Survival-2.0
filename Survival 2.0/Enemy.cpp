#include "Enemy.h"
#include <iostream>
#include <cmath>
#include "Player.h"
#include "Collision.h"

Enemy::Enemy(float sp)
    : speed(sp), sprite(idleTexture), health(100), maxHealth(100), size({ 100,100 }),
    attackCooldown(1.0f), attackClock(), animationTimer(0.0f), currentFrame(0),
    frameTime(0.15f), totalFrames(6), wasMoving(false), soundSystem(nullptr)
{
    boundingRectangle.setFillColor(sf::Color::Transparent);
    boundingRectangle.setOutlineColor(sf::Color::Red);
    boundingRectangle.setOutlineThickness(1.0f);
    boundingRectangle.setSize(sf::Vector2f(size.x / 4.0f, size.y / 4.0f));


    healthBarBackground.setSize(sf::Vector2f(30.0f, 6.0f));
    healthBarBackground.setFillColor(sf::Color::Red);
    healthBarBackground.setOutlineColor(sf::Color::Black);
    healthBarBackground.setOutlineThickness(1.0f);

    healthBarForeground.setSize(sf::Vector2f(30.0f, 6.0f));
    healthBarForeground.setFillColor(sf::Color::Green);
}

Enemy::~Enemy() {}

bool Enemy::Load(const sf::Vector2f pos)
{
    if (!idleTexture.loadFromFile("Assets/Player/Texture/Orc_idle.png")) {
        std::cout << "Failed to load idle texture!" << std::endl;
        return false;
    }

    if (!walkTexture.loadFromFile("Assets/Player/Texture/Orc_walk.png")) {
        std::cout << "Failed to load walk texture!" << std::endl;
        return false;
    }

    sprite.setTexture(idleTexture);
    sprite.setPosition(pos);
    sprite.setTextureRect(sf::IntRect(0, 0, size.x, size.y));
    sprite.setScale(1.8f, 1.8f);
    sprite.setOrigin(size.x / 2.0f, size.y / 2.0f);
    boundingRectangle.setOrigin(size.x / 8.0f, size.y / 8.0f);


    sf::Vector2f healthBarPos(pos.x - 15.0f, pos.y - 80.0f);
    healthBarBackground.setPosition(healthBarPos);
    healthBarForeground.setPosition(healthBarPos);

    return true;
}

void Enemy::Update(float deltaTime, const sf::Vector2f& playerPos, Player& player, const std::vector<int>& tiles)
{
    if (health <= 0) return;

    sf::Vector2f direction = playerPos - sprite.getPosition();
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    bool moving = (length > 0.0f);

    if (moving) direction /= length;

    sf::Vector2f velocity = direction * speed * deltaTime;

    sf::FloatRect testX = boundingRectangle.getGlobalBounds();
    testX.left += velocity.x;
    if (!collision.checkCollision(testX, tiles)) sprite.move(velocity.x, 0.0f);

    sf::FloatRect testY = boundingRectangle.getGlobalBounds();
    testY.top += velocity.y;
    if (!collision.checkCollision(testY, tiles)) sprite.move(0.0f, velocity.y);

    boundingRectangle.setPosition(sprite.getPosition());

    sf::Vector2f healthBarPos(sprite.getPosition().x - 15.0f, sprite.getPosition().y - 30.0f);
    healthBarBackground.setPosition(healthBarPos);
    healthBarForeground.setPosition(healthBarPos);


    float healthPercentage = static_cast<float>(health) / static_cast<float>(maxHealth);
    healthBarForeground.setSize(sf::Vector2f(30.0f * healthPercentage, 6.0f));

    if (healthPercentage > 0.6f) {
        healthBarForeground.setFillColor(sf::Color::Green);
    }
    else if (healthPercentage > 0.3f) {
        healthBarForeground.setFillColor(sf::Color::Yellow);
    }
    else if (healthPercentage > 0.9f) {
        healthBarForeground.setFillColor(sf::Color::Red);
    }
    else
    {

    }

    if (length < 40.0f) {
        if (attackClock.getElapsedTime().asSeconds() >= attackCooldown) {
            if (player.getGlobalBounds().intersects(sprite.getGlobalBounds())) {
                if (soundSystem) soundSystem->playSound("axeHit", 85.0f);
                player.ChangeHealth(-10);
                attackClock.restart();
            }
        }
    }

    if (moving) {
        animationTimer += deltaTime / 1000.0f;
        if (animationTimer >= frameTime) {
            animationTimer = 0.0f;
            currentFrame = (currentFrame + 1) % totalFrames;
        }

        sprite.setTexture(walkTexture);
        if (direction.x < 0.0f) {
            sprite.setScale(1.8f, 1.8f);
            sprite.setTextureRect(sf::IntRect((currentFrame + 1) * size.x, 0, -size.x, size.y));
        }
        else {
            sprite.setScale(1.8f, 1.8f);
            sprite.setTextureRect(sf::IntRect(currentFrame * size.x, 0, size.x, size.y));
        }
        wasMoving = true;
    }
    else if (wasMoving) {
        sprite.setTexture(idleTexture);
        sprite.setTextureRect(sf::IntRect(0, 0, size.x, size.y));
        currentFrame = 0;
        animationTimer = 0.0f;
        wasMoving = false;
    }
}

void Enemy::Draw(sf::RenderWindow& window)
{
    if (health > 0) {
        window.draw(sprite);
        window.draw(boundingRectangle);


        window.draw(healthBarBackground);
        window.draw(healthBarForeground);
    }
}

void Enemy::ChangeHealth(int hp)
{
    health += hp;
    if (health < 0) health = 0;
    if (health > maxHealth) health = maxHealth;
}