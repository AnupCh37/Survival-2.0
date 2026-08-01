#pragma once
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>

class Sound
{
private:
    std::unordered_map<std::string, sf::SoundBuffer> soundBuffers;
    std::unordered_map<std::string, sf::Sound> sounds;

public:
    Sound();
    ~Sound();

    bool loadSound(const std::string& name, const std::string& filename);
    void playSound(const std::string& name, float volume = 80.0f, float pitch = 0.8f);
    void stopSound(const std::string& name);
    void setVolume(const std::string& name, float volume);
    void setPitch(const std::string& name, float pitch);
    bool isPlaying(const std::string& name);
};