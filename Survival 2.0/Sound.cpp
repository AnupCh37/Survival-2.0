#include "Sound.h"
#include <iostream>

Sound::Sound()
{
    std::cout << "Sound system initialized." << std::endl;
}

Sound::~Sound()
{

    for (auto& soundPair : sounds)
    {
        soundPair.second.stop();
    }
    std::cout << "Sound system destroyed." << std::endl;
}

bool Sound::loadSound(const std::string& name, const std::string& filename)
{
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(filename))
    {
        std::cout << "Failed to load sound: " << filename << std::endl;
        return false;
    }

    soundBuffers[name] = buffer;
    sounds[name].setBuffer(soundBuffers[name]);

    std::cout << "Sound loaded successfully: " << name << " from " << filename << std::endl;
    return true;
}

void Sound::playSound(const std::string& name, float volume, float pitch)
{
    auto it = sounds.find(name);
    if (it != sounds.end())
    {
        it->second.setVolume(volume);
        it->second.setPitch(pitch);
        it->second.play();
    }
    else
    {
        std::cout << "Sound not found: " << name << std::endl;
    }
}

void Sound::stopSound(const std::string& name)
{
    auto it = sounds.find(name);
    if (it != sounds.end())
    {
        it->second.stop();
    }
}

void Sound::setVolume(const std::string& name, float volume)
{
    auto it = sounds.find(name);
    if (it != sounds.end())
    {
        it->second.setVolume(volume);
    }
}

void Sound::setPitch(const std::string& name, float pitch)
{
    auto it = sounds.find(name);
    if (it != sounds.end())
    {
        it->second.setPitch(pitch);
    }
}

bool Sound::isPlaying(const std::string& name)
{
    auto it = sounds.find(name);
    if (it != sounds.end())
    {
        return it->second.getStatus() == sf::Sound::Playing;
    }
    return false;
}
