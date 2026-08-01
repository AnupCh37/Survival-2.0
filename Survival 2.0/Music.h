

#pragma once
#include <SFML/Audio.hpp>
class Music
{
private:
	sf::Music backgroundMusic;
public:
	bool load(const std::string& filename);
	void play(bool loop = true);
	void stop();
};