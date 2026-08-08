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
		if (!wavePrepared) {
			prepareWave(currentWave);
			waveState = WaveState::SpawningEnemies;
		}
		break;

	case WaveState::SpawningEnemies:
		spawnAccumulator += deltaSeconds;
		if (!enemySpawnQueue.empty() && spawnAccumulator >= spawnInterval) {
			const EnemySpawnRequest& req = enemySpawnQueue.front();
			std::cout << "[QUEUE FRONT] Processing request " << req.requestId << std::endl;

			spawnNextEnemy(enemies);

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
			interWaveTimer.start(interWaveDelay);
			waveState = WaveState::WaitingForNextWave;
			std::cout << "[WAVE COMPLETE] Wave " << currentWave << " complete. Starting inter-wave countdown." << std::endl;
		}
		break;
	}

	case WaveState::WaitingForNextWave:
		interWaveTimer.update(deltaSeconds);
		if (!interWaveTimer.isActive()) {
			currentWave++;

			enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](const Enemy& e) {
				return e.getHealth() <= 0;
				}), enemies.end());

			spawnAccumulator = 0.0f;
			prepareWave(currentWave);
			waveState = WaveState::SpawningEnemies;
			std::cout << "[WAVE START] Preparing Wave " << currentWave << std::endl;
		}
		break;

	case WaveState::Finished:
		break;
	}
}

float WaveManager::getPlayerSpeed() const
{
	return 150.0f + static_cast<float>(currentWave - 1) * 10.0f;
}

void WaveManager::prepareWave(int waveNumber)
{
	if (wavePrepared && !enemySpawnQueue.empty()) return;

	while (!enemySpawnQueue.empty()) enemySpawnQueue.pop();

	// Wave 1 = 1 enemy, Wave 2 = 2 enemies, Wave 3 = 3 enemies, etc.
	int enemyCount = waveNumber;

	float baseSpeed = 0.10f + static_cast<float>(waveNumber - 1) * 0.01f;
	baseSpeed = std::min(baseSpeed, 0.25f);

	for (int i = 0; i < enemyCount; ++i) {
		EnemySpawnRequest req;
		req.requestId = nextRequestId++;
		req.waveNumber = waveNumber;
		req.speed = baseSpeed + static_cast<float>(i % 3) * 0.005f;
		req.spawnPosition = spawnPositions[i % spawnPositions.size()];

		enemySpawnQueue.push(req);
		std::cout << "[QUEUE PUSH] Wave " << waveNumber
			<< " | Request " << req.requestId
			<< " | Queue size: " << enemySpawnQueue.size() << std::endl;
	}

	wavePrepared = true;
}

void WaveManager::spawnNextEnemy(std::vector<Enemy>& enemies)
{
	if (enemySpawnQueue.empty()) return;

	const EnemySpawnRequest request = enemySpawnQueue.front();

	enemies.reserve(enemies.size() + enemySpawnQueue.size());

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