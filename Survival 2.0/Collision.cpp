#include "Collision.h"

int Collision::getTileAtPosition(const sf::Vector2f& position, const std::vector<int>& collisionData)
{
    int tileSize = 16;
    int widthInTiles = 50;
    int x = static_cast<int>(position.x) / tileSize;
    int y = static_cast<int>(position.y) / tileSize;
    int index = x + y * widthInTiles;
    if (index < 0 || index >= static_cast<int>(collisionData.size()))
        return -1;
    return collisionData[index];
}

bool Collision::checkCollision(const sf::FloatRect& bounds, const std::vector<int>& collisionData)
{
    int tileSize = 16;
    int mapWidth = 50;

    int startX = static_cast<int>(bounds.left) / tileSize;
    int endX = static_cast<int>(bounds.left + bounds.width) / tileSize;
    int startY = static_cast<int>(bounds.top) / tileSize;
    int endY = static_cast<int>(bounds.top + bounds.height) / tileSize;

    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            int index = x + y * mapWidth;
            if (index >= 0 && index < static_cast<int>(collisionData.size())) {
                int tileID = collisionData[index];
                if (collide.contains(tileID)) {
                    return true;
                }
            }
        }
    }

    return false;
}