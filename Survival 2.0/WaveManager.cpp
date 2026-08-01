#include "WaveManager.h"
#include <iostream>

WaveManager::WaveManager(Sound* soundSys)
	: currentWave(1), nextRequestId(1), totalEnemiesSpawned(0), soundSystem(soundSys)
{
	// Default spawn positions around the map (easy to change)
	spawnPositions = {
		{80.0f, 80.0f},
		{720.0f, 80.0f},
		{80.0f, 520.0f},
		{720.0f, 520.0f},
		{400.0f, 80.0f},
		{400.0f, 520.0f}
	};

	// Prepare Wave 1 exactly once on construction
	prepareWave(currentWave);
	waveState = WaveState::SpawningEnemies;
}

// Convert incoming delta milliseconds to seconds and drive state machine
void WaveManager::Update(float deltaTimeMilliseconds, std::vector<Enemy>& enemies)
{
	const float deltaSeconds = deltaTimeMilliseconds / 1000.0f;

	switch (waveState) {
	case WaveState::PreparingWave:
		// Should not re-prepare every frame; prepareWave flips wavePrepared
		if (!wavePrepared) {
			prepareWave(currentWave);
			waveState = WaveState::SpawningEnemies;
		}
		break;

	case WaveState::SpawningEnemies:
		// Spawn at most one enemy per spawn interval
		spawnAccumulator += deltaSeconds;
		if (!enemySpawnQueue.empty() && spawnAccumulator >= spawnInterval) {
			// Log which request we are about to process
			const EnemySpawnRequest& req = enemySpawnQueue.front();
			std::cout << "[QUEUE FRONT] Processing request " << req.requestId << std::endl;

			spawnNextEnemy(enemies);

			// reduce accumulator by one interval (allows slight catch-up)
			spawnAccumulator -= spawnInterval;
			if (spawnAccumulator < 0.0f) spawnAccumulator = 0.0f;
		}

		if (enemySpawnQueue.empty()) {
			std::cout << "[QUEUE EMPTY] All enemies for Wave " << currentWave << " have spawned" << std::endl;
			waveState = WaveState::Fighting;
		}
		break;

	case WaveState::Fighting: {
		int alive = countAliveEnemies(enemies);
		if (alive == 0 && enemySpawnQueue.empty()) {
			// Start inter-wave countdown
			interWaveTimer.start(interWaveDelay);
			waveState = WaveState::WaitingForNextWave;
			std::cout << "[WAVE COMPLETE] Wave " << currentWave << " complete. Starting inter-wave countdown." << std::endl;
		}
		break; }

	case WaveState::WaitingForNextWave:
		// Advance the countdown
		interWaveTimer.update(deltaSeconds);
		if (!interWaveTimer.isActive()) {
			// Move to the next wave
			currentWave++;

			// Clear dead enemies from the vector (keep alive ones)
			enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](const Enemy& e) {
				return e.getHealth() <= 0;
			}), enemies.end());

			// Prepare and begin spawning the new wave
			spawnAccumulator = 0.0f;
			prepareWave(currentWave);
			waveState = WaveState::SpawningEnemies;
			std::cout << "[WAVE START] Preparing Wave " << currentWave << std::endl;
		}
		break;

	case WaveState::Finished:
		// No-op for now
		break;
	}
}

float WaveManager::getPlayerSpeed() const
{
	return 150.0f + static_cast<float>(currentWave - 1) * 10.0f;
}

// Create lightweight spawn requests and push them into the queue (no Enemy objects)
void WaveManager::prepareWave(int waveNumber)
{
	// Prevent refilling repeatedly
	if (wavePrepared && !enemySpawnQueue.empty()) return;

	// Clear any previous queued requests
	while (!enemySpawnQueue.empty()) enemySpawnQueue.pop();

	int enemyCount = 2 + waveNumber; // scalable formula
	float baseSpeed = 0.10f + static_cast<float>(waveNumber - 1) * 0.01f;
	baseSpeed = std::min(baseSpeed, 0.25f);

	for (int i = 0; i < enemyCount; ++i) {
		EnemySpawnRequest req;
		req.requestId = nextRequestId++;
		req.waveNumber = waveNumber;
		req.speed = baseSpeed + static_cast<float>(i % 3) * 0.005f;
		req.spawnPosition = spawnPositions[i % spawnPositions.size()];

		enemySpawnQueue.push(req);
		std::cout << "[QUEUE PUSH] Wave " << waveNumber << " | Request " << req.requestId << " | Queue size: " << enemySpawnQueue.size() << std::endl;
	}

	wavePrepared = true;
}

// Spawn the single front request into the provided enemies vector
void WaveManager::spawnNextEnemy(std::vector<Enemy>& enemies)
{
	if (enemySpawnQueue.empty()) return;

	const EnemySpawnRequest request = enemySpawnQueue.front();

	// Reserve capacity to avoid reallocation and expensive copies of SFML objects
	enemies.reserve(enemies.size() + enemySpawnQueue.size());

	// Construct in-place to avoid copies where possible
	enemies.emplace_back(request.speed);
	Enemy& newEnemy = enemies.back();

	if (newEnemy.Load(request.spawnPosition)) {
		newEnemy.soundSystem = soundSystem;
		totalEnemiesSpawned++;

		std::cout << "[QUEUE POP] Request " << request.requestId << " removed" << std::endl;
		enemySpawnQueue.pop();
		std::cout << "[QUEUE SIZE] " << enemySpawnQueue.size() << " requests remaining" << std::endl;
	}
	else {
		// Failed to load enemy: remove and report
		std::cerr << "Failed to spawn queued enemy request " << request.requestId << '\n';
		enemySpawnQueue.pop();
	}
}

int WaveManager::countAliveEnemies(const std::vector<Enemy>& enemies) const
{
	int alive = 0;
	for (const auto& e : enemies) {
		if (e.getHealth() > 0) ++alive;
	}
	return alive;
}

bool WaveManager::isWaveLoading() const
{
	return waveState == WaveState::WaitingForNextWave && interWaveTimer.isActive();
}

float WaveManager::getTimeRemaining() const
{
	if (waveState == WaveState::WaitingForNextWave) return interWaveTimer.getTimeRemaining();
	return 0.0f;
}
