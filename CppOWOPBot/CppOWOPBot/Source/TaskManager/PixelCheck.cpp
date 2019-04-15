#include "PixelCheck.h"

void PixelCheck::AddPixelToCheck(const sf::Vector2i & worldPos)
{
	std::lock_guard<std::mutex> lock(mMutex);
	mPixels.push_back(worldPos);
}

void PixelCheck::CheckPixelsAndAddErrorTasks()
{
	std::lock_guard<std::mutex> lock(mMutex);

	//TODO: add less world mutex locking eg use .Update
	for(sf::Vector2i &pos : mPixels)
	{
		sf::Color frontCol = mWorld.GetPixel(pos, Chunk::BufferType::Front);
		sf::Color backCol = mWorld.GetPixel(pos, Chunk::BufferType::Back);
		if (frontCol != backCol)
		{
			mWorld.SetPixel(pos, frontCol, Chunk::BufferType::Back);
			mTaskManager.PushTaskFront(Task::Task(Task::Type::PlacePixel, pos, backCol));
		}
	}

	ClearPixelsUnsafe();
}

void PixelCheck::ClearPixels()
{
	std::lock_guard<std::mutex> lock(mMutex);
	ClearPixelsUnsafe();
}

void PixelCheck::ClearPixelsUnsafe()
{
	mPixels.clear();
	if (mPixels.capacity() > 1024 * 16) //increaces performance(?), only shrinks if vector is bigger than 64kb
		mPixels.shrink_to_fit();
}
