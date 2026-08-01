#include "Music.h"

bool Music::load(const std::string& filename)
{
    return backgroundMusic.openFromFile(filename);
}

void Music::play(bool loop)
{
    backgroundMusic.setLoop(loop);
    backgroundMusic.play();
    backgroundMusic.setPitch(2.0f);
    backgroundMusic.setVolume(20.0f);
}

void Music::stop()
{
    backgroundMusic.stop();
}