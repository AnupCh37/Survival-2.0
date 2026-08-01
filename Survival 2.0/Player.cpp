#define _USE_MATH_DEFINES
#include "Player.h"
#include "Math.h"
#include <iostream>
#include <string>
#include <cmath>
#include "Enemy.h"

int getTileAtPosition(const sf::Vector2f& pos, const std::vector<int>& level)
{
    const int tilew = 16;
    const int inttileh = 16;
    const int mapw = 50;
    const int maph = 38;
    int x = static_cast<int>(pos.x) / tilew;
    int y = static_cast<int>(pos.y) / inttileh;

    if (x < 0 || x >= mapw || y < 0 || y >= maph)
        return -1;

    return level[y * mapw + x];
}

Player::Player() :
    idleTexture(), attackTexture(), walkTexture(), psprite(),
    size({ 100, 100 }),
    boneSpeed(0.4f),
    boneFireRate(0.3f),
    health(100),
    healthText(),
    animationTimer(0.0f),
    currentFrame(0),
    frameTime(0.1f),
    totalFrames(6),
    soundSystem(nullptr),
    wasMoving(false)
{
    boundingRectangle.setFillColor(sf::Color::Transparent);
    boundingRectangle.setOutlineColor(sf::Color::Blue);
    boundingRectangle.setOutlineThickness(1.0f);
    boundingRectangle.setSize(sf::Vector2f(32.0f, 32.0f));
    psprite.setOrigin(size.x / 2.0f, size.y / 2.0f);

    healthText.setCharacterSize(20);
    healthText.setFillColor(sf::Color::Green);
    healthText.setString(std::to_string(health));
}

Player::~Player()
{
}

void Player::ChangeHealth(int hp) {
    int oldHealth = health;
    health += hp;
    if (health < 0)
        health = 0;
    healthText.setString(std::to_string(health));

    if (hp < 0 && soundSystem) {
        soundSystem->playSound("playerHurt", 80.0f);
    }
    if (oldHealth > 0 && health == 0 && soundSystem) {
        soundSystem->playSound("playerDeath", 100.0f);
    }
    std::cout << "Player Health Changed to: " << health << std::endl;
}

void Player::Load()
{
    if (idleTexture.loadFromFile("assets/Player/Texture/idle.png"))
    {
        psprite.setTexture(idleTexture);
        psprite.setTextureRect({ 0, 0 , size.x, size.y });
        psprite.setScale(2.0f, 2.0f);
        boundingRectangle.setSize(sf::Vector2f(static_cast<float>(size.x) / 4.0f, static_cast<float>(size.y) / 4.0f));
        boundingRectangle.setPosition(400.0f, 300.0f);
        boundingRectangle.setOrigin(sf::Vector2f(static_cast<float>(size.x) / 8.0f, static_cast<float>(size.y) / 8.0f));
        psprite.setPosition(400.0f, 300.0f);
    }
    else
    {
        std::cout << "Player image cannot be loaded\n";
    }

    if (!boneTexture.loadFromFile("Assets/GunAssets/PNG/arrow.png")) {
        std::cout << "Failed to load Bone texture.\n";
    }

    if (font.loadFromFile("Assets/Fonts/arial.ttf")) {
        healthText.setFont(font);
        healthText.setCharacterSize(16);
    }
    else {
        std::cout << "Failed to load font for player health.\n";
    }
}

void Player::Update(float deltaTime, std::vector<Enemy>& enemies, sf::RenderWindow& window, const std::vector<int>& tiles, float speed)
{
    sf::Vector2f velocity(0.0f, 0.0f);
    float baseSpeed = speed;
    bool Moving = false;
    float moveAmount = baseSpeed * deltaTime / 1000.0f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        velocity.y -= moveAmount;
        Moving = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        velocity.y += moveAmount;
        Moving = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        velocity.x -= moveAmount;
        psprite.setScale(-2.0f, 2.0f);
        Moving = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        velocity.x += moveAmount;
        psprite.setScale(2.0f, 2.0f);
        Moving = true;
    }

    if (Moving) {
        if (!wasMoving && soundSystem) {
            soundSystem->playSound("playerWalk", 60.0f, 1.2f);
        }
        wasMoving = true;

        animationTimer += deltaTime / 1000.0f;
        if (animationTimer >= frameTime) {
            animationTimer = 0.0f;
            currentFrame = (currentFrame + 1) % totalFrames;
            walkTexture.loadFromFile("assets/Player/Texture/walk.png");
            psprite.setTexture(walkTexture);
            psprite.setTextureRect(sf::IntRect(currentFrame * 100, 0, 100, 100));
        }
    }
    else {
        if (wasMoving && soundSystem) {
            soundSystem->stopSound("playerWalk");
        }
        wasMoving = false;

        if (currentFrame != 0) {
            currentFrame = 0;
            animationTimer = 0.0f;
            psprite.setTexture(idleTexture);
            psprite.setTextureRect({ 0, 0, 100, 100 });
        }
    }

    sf::FloatRect bounds = boundingRectangle.getGlobalBounds();

    sf::FloatRect testXBounds = bounds;
    testXBounds.left += velocity.x;
    bool canMoveX = !collision.checkCollision(testXBounds, tiles);

    sf::FloatRect testYBounds = bounds;
    testYBounds.top += velocity.y;
    bool canMoveY = !collision.checkCollision(testYBounds, tiles);

    if (canMoveX) psprite.move(velocity.x, 0.0f);
    if (canMoveY) psprite.move(0.0f, velocity.y);
    if (canMoveX) boundingRectangle.move(velocity.x, 0.0f);
    if (canMoveY) boundingRectangle.move(0.0f, velocity.y);

    healthText.setPosition(
        boundingRectangle.getPosition().x - healthText.getLocalBounds().width / 2.0f,
        boundingRectangle.getPosition().y - healthText.getLocalBounds().height / 2.0f - 35.0f
    );

    sf::Vector2f mouseWorldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && boneFireClock.getElapsedTime().asSeconds() > boneFireRate)
    {
        if (soundSystem) {
            soundSystem->playSound("arrowShoot", 70.0f, 1.1f);
        }

        sf::Sprite newBone(boneTexture);
        newBone.setPosition(psprite.getPosition());
        newBone.setOrigin(newBone.getLocalBounds().width / 2.0f, newBone.getLocalBounds().height / 2.0f);
        newBone.setScale(1.15f, 1.15f);

        sf::Vector2f direction = Math::NormalizeVector(mouseWorldPos - psprite.getPosition());
        float rotationAngleRad = std::atan2(direction.y, direction.x);
        newBone.setRotation(rotationAngleRad * 180.0f / static_cast<float>(M_PI));


        sf::RectangleShape boneRect(sf::Vector2f(newBone.getGlobalBounds().width / 2.25f, newBone.getGlobalBounds().height / 2.25f));
        boneRect.setFillColor(sf::Color::Transparent);
        boneRect.setOutlineColor(sf::Color::Transparent);
        boneRect.setOutlineThickness(1.0f);
        boneRect.setOrigin(boneRect.getSize() / 2.0f);
        boneRect.setPosition(newBone.getPosition());

        bones.push_back(newBone);
        boneDirections.push_back(direction);
        boneBounds.push_back(boneRect);

        boneFireClock.restart();
    }


    for (size_t i = 0; i < bones.size();) {
        sf::Vector2f movement = boneDirections[i] * boneSpeed * deltaTime;
        bones[i].move(movement);
        boneBounds[i].move(movement);

        bool hit = false;

        sf::FloatRect arrowBounds = boneBounds[i].getGlobalBounds();
        if (collision.checkCollision(arrowBounds, tiles)) {
            bones.erase(bones.begin() + i);
            boneDirections.erase(boneDirections.begin() + i);
            boneBounds.erase(boneBounds.begin() + i);
            hit = true;
        }

        if (!hit) {
            for (auto& enemy : enemies) {
                if (enemy.getHealth() > 0 && Math::DidRectCollision(arrowBounds, enemy.boundingRectangle.getGlobalBounds())) {
                    enemy.ChangeHealth(-10);
                    std::cout << "Enemy hit! Current Enemy Health: " << enemy.getHealth() << std::endl;
                    bones.erase(bones.begin() + i);
                    boneDirections.erase(boneDirections.begin() + i);
                    boneBounds.erase(boneBounds.begin() + i);
                    hit = true;
                    break;
                }
            }
        }

        if (!hit) {
            sf::Vector2u windowSize = window.getSize();
            if (arrowBounds.left > windowSize.x || arrowBounds.left + arrowBounds.width < 0 ||
                arrowBounds.top > windowSize.y || arrowBounds.top + arrowBounds.height < 0) {
                bones.erase(bones.begin() + i);
                boneDirections.erase(boneDirections.begin() + i);
                boneBounds.erase(boneBounds.begin() + i);
            }
            else {
                ++i;
            }
        }
    }
}

void Player::Draw(float deltaTime, sf::RenderWindow& window)
{
    window.draw(psprite);
    window.draw(boundingRectangle);

    for (const auto& bone : bones) {
        window.draw(bone);
    }

    for (const auto& bound : boneBounds) {
        window.draw(bound);
    }

    window.draw(healthText);
}