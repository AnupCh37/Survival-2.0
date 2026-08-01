#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <queue>
#include <vector>
#include <algorithm>
#include "Enemy.h"
#include "Sound.h"

// Simple delay timer used for the inter-wave countdown. Uses delta accumulation.
class DelayTimer {
private:
    float remainingSeconds = 0.0f;
    bool active = false;

public:
    DelayTimer() = default;

    void start(float seconds) {
        remainingSeconds = seconds;
        active = true;
    }

    // Advance the timer by deltaSeconds; stop when reaches zero
    void update(float deltaSeconds) {
        if (!active) return;
        remainingSeconds -= deltaSeconds;
        if (remainingSeconds <= 0.0f) {
            remainingSeconds = 0.0f;
            active = false;
        }
    }

    float getTimeRemaining() const { return remainingSeconds; }

    bool isActive() const { return active; }
    bool isFinished() const { return !active && remainingSeconds <= 0.0f; }
};

// Enemy spawn request stored in the FIFO queue. Lightweight: no textures or sprites.
struct EnemySpawnRequest {
    int requestId;
    int waveNumber;
    float speed;
    sf::Vector2f spawnPosition;
};

enum class WaveState {
    PreparingWave,
    SpawningEnemies,
    Fighting,
    WaitingForNextWave,
    Finished
};

class WaveManager {
private:
    // Queue of lightweight spawn requests demonstrating FIFO operations
    std::queue<EnemySpawnRequest> enemySpawnQueue;

    int currentWave = 1;
    int nextRequestId = 1;
    int totalEnemiesSpawned = 0; // enemies actually instantiated in-game

    // Timers and intervals (seconds)
    float spawnInterval = 0.75f; // seconds between spawns
    float spawnAccumulator = 0.0f;
    float interWaveDelay = 3.0f; // seconds to wait between waves
    DelayTimer interWaveTimer;

    bool wavePrepared = false;
    WaveState waveState = WaveState::PreparingWave;

    Sound* soundSystem = nullptr;

    std::vector<sf::Vector2f> spawnPositions;

    // Internal helpers
    void prepareWave(int waveNumber);
    void spawnNextEnemy(std::vector<Enemy>& enemies);
    int countAliveEnemies(const std::vector<Enemy>& enemies) const;

public:
    WaveManager(Sound* soundSys);

    // Update called with deltaTime in milliseconds (main provides ms)
    void Update(float deltaTimeMilliseconds, std::vector<Enemy>& enemies);

    // Accessors required by main UI and game logic
    int getCurrentWave() const { return currentWave; }
    float getPlayerSpeed() const;
    bool isWaveLoading() const; // true while waiting between waves
    float getTimeRemaining() const; // seconds remaining for next wave when waiting
    std::size_t getQueuedEnemyCount() const { return enemySpawnQueue.size(); }
    bool isSpawnQueueEmpty() const { return enemySpawnQueue.empty(); }
    int getAliveEnemyCount(const std::vector<Enemy>& enemies) const { return countAliveEnemies(enemies); }
    WaveState getWaveState() const { return waveState; }
    int getTotalEnemiesSpawned() const { return totalEnemiesSpawned; }
};
