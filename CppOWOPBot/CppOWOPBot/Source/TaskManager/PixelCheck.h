#pragma once
#include <vector> //i think vector is faster than deque
#include <SFML/Graphics.hpp>
#include <mutex>
#include "TaskManager.h"
#include "World/World.h"

//will compare Front and Backbuffer for specified pixels and if they dont match the pixel gets readded to the taskmanager
class PixelCheck
{
public:
	PixelCheck(TaskManager &taskmanager, World& world) : mTaskManager(taskmanager),mWorld(world) {}

	void AddPixelToCheck(const sf::Vector2i &worldPos);
	void CheckPixelsAndAddErrorTasks(); //long func name kek
	void ClearPixels();

private:

	void ClearPixelsUnsafe();

	mutable std::mutex mMutex;

	std::vector<sf::Vector2i> mPixels;

	World &mWorld;
	TaskManager &mTaskManager;
};