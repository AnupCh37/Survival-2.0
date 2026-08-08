#include "PathFinding.h"
#include "Pathfinding.h"
#include "Collision.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <queue>

namespace
{
    struct OpenNode
    {
        sf::Vector2i tile;
        int fScore;

        bool operator<(const OpenNode& other) const
        {
            return fScore > other.fScore;
        }
    };
}

sf::Vector2i Pathfinding::worldToTile(const sf::Vector2f& worldPosition) const
{
    return sf::Vector2i(
        static_cast<int>(std::floor(worldPosition.x / static_cast<float>(TILE_SIZE))),
        static_cast<int>(std::floor(worldPosition.y / static_cast<float>(TILE_SIZE)))
    );
}

sf::Vector2f Pathfinding::tileToWorldCenter(const sf::Vector2i& tile) const
{
    return sf::Vector2f(
        tile.x * static_cast<float>(TILE_SIZE) + TILE_SIZE * 0.5f,
        tile.y * static_cast<float>(TILE_SIZE) + TILE_SIZE * 0.5f
    );
}

bool Pathfinding::isInsideMap(const sf::Vector2i& tile, const std::vector<int>& collisionTiles) const
{
    if (tile.x < 0 || tile.y < 0 || tile.x >= MAP_WIDTH || tile.y >= MAP_HEIGHT)
        return false;

    const int index = tile.x + tile.y * MAP_WIDTH;
    return index >= 0 && index < static_cast<int>(collisionTiles.size());
}

bool Pathfinding::isTileWalkable(
    const sf::Vector2i& tile,
    const std::vector<int>& collisionTiles,
    const sf::Vector2f& enemyCollisionSize) const
{
    if (!isInsideMap(tile, collisionTiles))
        return false;

    Collision collision;
    const sf::Vector2f center = tileToWorldCenter(tile);
    sf::FloatRect bounds(
        center.x - enemyCollisionSize.x * 0.5f,
        center.y - enemyCollisionSize.y * 0.5f,
        enemyCollisionSize.x,
        enemyCollisionSize.y
    );

    return !collision.checkCollision(bounds, collisionTiles);
}

sf::Vector2i Pathfinding::findNearestWalkableTile(
    const sf::Vector2i& origin,
    const std::vector<int>& collisionTiles,
    const sf::Vector2f& enemyCollisionSize,
    int maxRadius) const
{
    if (isTileWalkable(origin, collisionTiles, enemyCollisionSize))
        return origin;

    std::queue<std::pair<sf::Vector2i, int>> frontier;
    std::vector<bool> visited(MAP_WIDTH * MAP_HEIGHT, false);

    if (isInsideMap(origin, collisionTiles)) {
        frontier.push({ origin, 0 });
        visited[origin.x + origin.y * MAP_WIDTH] = true;
    }
    else {
        sf::Vector2i clamped(
            std::max(0, std::min(MAP_WIDTH - 1, origin.x)),
            std::max(0, std::min(MAP_HEIGHT - 1, origin.y))
        );
        frontier.push({ clamped, 0 });
        visited[clamped.x + clamped.y * MAP_WIDTH] = true;
    }

    const sf::Vector2i directions[4] = {
        { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }
    };

    while (!frontier.empty()) {
        const auto current = frontier.front();
        frontier.pop();

        if (current.second > maxRadius)
            continue;

        if (isTileWalkable(current.first, collisionTiles, enemyCollisionSize))
            return current.first;

        if (current.second == maxRadius)
            continue;

        for (const auto& direction : directions) {
            const sf::Vector2i next = current.first + direction;
            if (!isInsideMap(next, collisionTiles))
                continue;

            const int index = next.x + next.y * MAP_WIDTH;
            if (visited[index])
                continue;

            visited[index] = true;
            frontier.push({ next, current.second + 1 });
        }
    }

    return sf::Vector2i(-1, -1);
}

int Pathfinding::heuristic(const sf::Vector2i& a, const sf::Vector2i& b) const
{
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

std::vector<sf::Vector2i> Pathfinding::findPath(
    const sf::Vector2i& startTile,
    const sf::Vector2i& goalTile,
    const std::vector<int>& collisionTiles,
    const sf::Vector2f& enemyCollisionSize)
{
    std::cout << "[PATHFINDING] Search start " << startTile.x << "," << startTile.y
        << " -> " << goalTile.x << "," << goalTile.y << std::endl;

    sf::Vector2i validStart = startTile;
    sf::Vector2i validGoal = goalTile;

    if (!isTileWalkable(validStart, collisionTiles, enemyCollisionSize)) {
        validStart = findNearestWalkableTile(validStart, collisionTiles, enemyCollisionSize, 5);
    }

    if (!isTileWalkable(validGoal, collisionTiles, enemyCollisionSize)) {
        validGoal = findNearestWalkableTile(validGoal, collisionTiles, enemyCollisionSize, 5);
    }

    if (validStart.x < 0 || validStart.y < 0 || validGoal.x < 0 || validGoal.y < 0) {
        std::cout << "[PATHFINDING] No valid start/goal tile found" << std::endl;
        return {};
    }

    if (validStart == validGoal)
        return {};

    const int nodeCount = MAP_WIDTH * MAP_HEIGHT;
    const int infinity = std::numeric_limits<int>::max();
    std::vector<int> gScore(nodeCount, infinity);
    std::vector<sf::Vector2i> cameFrom(nodeCount, sf::Vector2i(-1, -1));
    std::vector<bool> closed(nodeCount, false);
    std::priority_queue<OpenNode> openSet;

    const int startIndex = validStart.x + validStart.y * MAP_WIDTH;
    gScore[startIndex] = 0;
    openSet.push({ validStart, heuristic(validStart, validGoal) });

    const sf::Vector2i directions[4] = {
        { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }
    };

    while (!openSet.empty()) {
        const sf::Vector2i current = openSet.top().tile;
        openSet.pop();

        const int currentIndex = current.x + current.y * MAP_WIDTH;
        if (closed[currentIndex])
            continue;
        closed[currentIndex] = true;

        if (current == validGoal) {
            std::vector<sf::Vector2i> path;
            sf::Vector2i cursor = validGoal;

            while (cursor != validStart) {
                path.push_back(cursor);
                const int cursorIndex = cursor.x + cursor.y * MAP_WIDTH;
                cursor = cameFrom[cursorIndex];
                if (cursor.x < 0 || cursor.y < 0)
                    return {};
            }

            std::reverse(path.begin(), path.end());
            std::cout << "[PATHFINDING] Path found with " << path.size() << " waypoint(s)" << std::endl;
            return path;
        }

        for (const auto& direction : directions) {
            const sf::Vector2i neighbor = current + direction;
            if (!isTileWalkable(neighbor, collisionTiles, enemyCollisionSize))
                continue;

            const int neighborIndex = neighbor.x + neighbor.y * MAP_WIDTH;
            if (closed[neighborIndex])
                continue;

            const int tentativeG = gScore[currentIndex] + 1;
            if (tentativeG >= gScore[neighborIndex])
                continue;

            cameFrom[neighborIndex] = current;
            gScore[neighborIndex] = tentativeG;
            openSet.push({ neighbor, tentativeG + heuristic(neighbor, validGoal) });
        }
    }

    std::cout << "[PATHFINDING] No path found" << std::endl;
    return {};
}