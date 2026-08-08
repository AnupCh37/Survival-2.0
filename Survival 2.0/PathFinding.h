#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class Pathfinding
{
public:
    std::vector<sf::Vector2i> findPath(
        const sf::Vector2i& startTile,
        const sf::Vector2i& goalTile,
        const std::vector<int>& collisionTiles,
        const sf::Vector2f& enemyCollisionSize);

    bool isTileWalkable(
        const sf::Vector2i& tile,
        const std::vector<int>& collisionTiles,
        const sf::Vector2f& enemyCollisionSize) const;

    sf::Vector2i findNearestWalkableTile(
        const sf::Vector2i& origin,
        const std::vector<int>& collisionTiles,
        const sf::Vector2f& enemyCollisionSize,
        int maxRadius = 5) const;

    sf::Vector2i worldToTile(const sf::Vector2f& worldPosition) const;
    sf::Vector2f tileToWorldCenter(const sf::Vector2i& tile) const;

private:
    static constexpr int TILE_SIZE = 16;
    static constexpr int MAP_WIDTH = 50;
    static constexpr int MAP_HEIGHT = 38;

    bool isInsideMap(const sf::Vector2i& tile, const std::vector<int>& collisionTiles) const;
    int heuristic(const sf::Vector2i& a, const sf::Vector2i& b) const;
};