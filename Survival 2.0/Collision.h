
#pragma once
#include <vector>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <set>
class Collision
{
public:
	bool checkCollision(const sf::FloatRect& bounds, const std::vector<int>& tiles);
	int getTileAtPosition(const sf::Vector2f& position, const std::vector<int>& tiles);
	std::set<int> collide = { 2,4,5,24,25,26,29,48,49,53,73,76,97,100,123,124,125,147,148,149 };
};