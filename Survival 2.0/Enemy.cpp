#include "Enemy.h"
#include <iostream>
#include <cmath>
#include "Player.h"
#include "Collision.h"

Enemy::Enemy(float sp)
    : speed(sp), sprite(idleTexture), health(100), maxHealth(100), size({ 100,100 }),
    attackCooldown(1.0f), attackClock(), animationTimer(0.0f), currentFrame(0),
    frameTime(0.15f), totalFrames(6), wasMoving(false), soundSystem(nullptr),
    currentPathIndex(0), pathUpdateAccumulator(0.4f), pathUpdateInterval(0.4f),
    hasValidPath(false), lastPlayerTile({ -1, -1 }), stuckAccumulator(0.0f), previousPosition({ 0.0f, 0.0f })
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
    boundingRectangle.setPosition(pos);
    previousPosition = pos;

    sf::Vector2f healthBarPos(pos.x - 15.0f, pos.y - 80.0f);
    healthBarBackground.setPosition(healthBarPos);
    healthBarForeground.setPosition(healthBarPos);

    return true;
}

void Enemy::Update(float deltaTime, const sf::Vector2f& playerPos, Player& player, const std::vector<int>& tiles)
{
    if (health <= 0) return;

    const float deltaTimeMilliseconds = deltaTime;
    const float deltaSeconds = deltaTime / 1000.0f;
    pathUpdateAccumulator += deltaSeconds;
    stuckAccumulator += deltaSeconds;

    if (hasValidPath && currentPathIndex >= currentPath.size())
        invalidatePath();

    const sf::Vector2i playerTile = pathfinding.worldToTile(playerPos);
    const sf::Vector2i enemyTile = pathfinding.worldToTile(sprite.getPosition());

    bool waypointBlocked = false;
    if (hasValidPath && currentPathIndex < currentPath.size()) {
        waypointBlocked = !pathfinding.isTileWalkable(
            currentPath[currentPathIndex], tiles, boundingRectangle.getSize());
    }

    bool stuck = false;
    if (stuckAccumulator >= 0.5f) {
        const sf::Vector2f displacement = sprite.getPosition() - previousPosition;
        const float distanceMoved = std::sqrt(
            displacement.x * displacement.x + displacement.y * displacement.y);

        stuck = hasValidPath && distanceMoved < 0.5f;
        previousPosition = sprite.getPosition();
        stuckAccumulator = 0.0f;
    }

    const bool playerChangedTile = playerTile != lastPlayerTile;
    const bool intervalElapsed = pathUpdateAccumulator >= pathUpdateInterval;
    const bool needsNewPath = !hasValidPath || playerChangedTile || waypointBlocked || stuck || intervalElapsed;

    if (needsNewPath) {
        std::vector<sf::Vector2i> newPath = pathfinding.findPath(
            enemyTile,
            playerTile,
            tiles,
            boundingRectangle.getSize());

        currentPath = newPath;
        currentPathIndex = 0;
        hasValidPath = !currentPath.empty();
        lastPlayerTile = playerTile;
        pathUpdateAccumulator = 0.0f;

        if (hasValidPath) {
            std::cout << "[ENEMY PATH] New path with " << currentPath.size()
                << " waypoint(s)." << std::endl;
        }
    }

    const sf::Vector2f positionBeforeMove = sprite.getPosition();
    bool actuallyMoved = false;
    if (hasValidPath) {
        actuallyMoved = moveTowardCurrentWaypoint(deltaTimeMilliseconds, tiles);
    }

    const sf::Vector2f movement = sprite.getPosition() - positionBeforeMove;
    const sf::Vector2f playerDirection = playerPos - sprite.getPosition();
    const float length = std::sqrt(
        playerDirection.x * playerDirection.x + playerDirection.y * playerDirection.y);

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

    if (length < 40.0f) {
        if (attackClock.getElapsedTime().asSeconds() >= attackCooldown) {
            if (player.getGlobalBounds().intersects(sprite.getGlobalBounds())) {
                if (soundSystem) soundSystem->playSound("axeHit", 85.0f);
                player.ChangeHealth(-10);
                attackClock.restart();
            }
        }
    }

    if (actuallyMoved) {
        animationTimer += deltaSeconds;
        if (animationTimer >= frameTime) {
            animationTimer = 0.0f;
            currentFrame = (currentFrame + 1) % totalFrames;
        }

        sprite.setTexture(walkTexture);
        if (movement.x < 0.0f) {
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

bool Enemy::moveTowardCurrentWaypoint(float deltaTimeMilliseconds, const std::vector<int>& collisionTiles)
{
    if (!hasValidPath || currentPathIndex >= currentPath.size())
        return false;

    const float alignmentTolerance = 1.5f;
    const float waypointTolerance = 3.0f;
    const float movementDistance = speed * deltaTimeMilliseconds;

    const sf::Vector2f waypoint = pathfinding.tileToWorldCenter(currentPath[currentPathIndex]);
    const sf::Vector2f position = sprite.getPosition();
    const float dx = waypoint.x - position.x;
    const float dy = waypoint.y - position.y;

    auto tryMoveX = [&](float amount) -> bool {
        if (std::abs(amount) <= alignmentTolerance)
            return false;

        const float stepX = std::copysign(std::min(std::abs(amount), movementDistance), amount);
        sf::FloatRect projectedBounds = boundingRectangle.getGlobalBounds();
        projectedBounds.left += stepX;

        if (collision.checkCollision(projectedBounds, collisionTiles))
            return false;

        sprite.move(stepX, 0.0f);
        boundingRectangle.setPosition(sprite.getPosition());
        return true;
        };

    auto tryMoveY = [&](float amount) -> bool {
        if (std::abs(amount) <= alignmentTolerance)
            return false;

        const float stepY = std::copysign(std::min(std::abs(amount), movementDistance), amount);
        sf::FloatRect projectedBounds = boundingRectangle.getGlobalBounds();
        projectedBounds.top += stepY;

        if (collision.checkCollision(projectedBounds, collisionTiles))
            return false;

        sprite.move(0.0f, stepY);
        boundingRectangle.setPosition(sprite.getPosition());
        return true;
        };

    bool moved = false;

    // Follow the dominant waypoint direction first. This prevents a small
    // off-axis alignment correction from repeatedly pushing into wall corners.
    if (std::abs(dx) >= std::abs(dy)) {
        moved = tryMoveX(dx);
        if (!moved)
            moved = tryMoveY(dy);
    }
    else {
        moved = tryMoveY(dy);
        if (!moved)
            moved = tryMoveX(dx);
    }

    if (!moved && (std::abs(dx) > alignmentTolerance || std::abs(dy) > alignmentTolerance)) {
        invalidatePath();
        pathUpdateAccumulator = pathUpdateInterval;
        return false;
    }

    const sf::Vector2f remaining = waypoint - sprite.getPosition();
    const float remainingDistance = std::sqrt(
        remaining.x * remaining.x + remaining.y * remaining.y);

    if (remainingDistance <= waypointTolerance) {
        sf::FloatRect snappedBounds = boundingRectangle.getGlobalBounds();
        snappedBounds.left += waypoint.x - sprite.getPosition().x;
        snappedBounds.top += waypoint.y - sprite.getPosition().y;

        if (!collision.checkCollision(snappedBounds, collisionTiles)) {
            sprite.setPosition(waypoint);
            boundingRectangle.setPosition(sprite.getPosition());
        }

        ++currentPathIndex;
        if (currentPathIndex >= currentPath.size())
            invalidatePath();
    }

    return moved;
}

void Enemy::invalidatePath()
{
    hasValidPath = false;
}

sf::Vector2f Enemy::getCollisionSize() const
{
    return boundingRectangle.getSize();
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