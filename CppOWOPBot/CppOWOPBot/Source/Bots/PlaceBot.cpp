#include "PlaceBot.h"
#include "BotManager.h"

void PlaceBot::Update(float dt)
{
	ConnectionBot::Update(dt);

	World &world = mManager.GetWorld();
	TaskManager &taskManager = mManager.GetTaskManager();

	if (mConnectionState >= ConnectionState::Joined)
	{
		for (int max = 100; max > 0 && mPlaceBucket.CanSpend(1) && taskManager.GetNumTasks(Task::PlacePixel); max--)
		{
			Task placeTask;
			while (taskManager.PopTask(Task::PlacePixel, placeTask))
			{
				sf::Vector2i chunkPos = world.WorldToChunk(placeTask.pos);
				
				if (!world.ChunkExists(chunkPos))
				{
					world.CreateChunk(chunkPos);
					taskManager.PushTask(Task::Task(Task::RequestChunk, chunkPos));
				}

				if (world.IsChunkLocked(chunkPos))
					continue;


				bool brick = false;
				world.Update([&,this](UnsafeWorld &uworld)
				{
					if (uworld.GetPixel(placeTask.pos, Chunk::BufferType::Back) != placeTask.color)
					{
						if (!PlacePixel(placeTask.pos, placeTask.color))
							taskManager.PushTask(placeTask);
						brick = true; //idk how else to do that ...
					}
				});

				if (brick)
					break;
			}
		}


		if (mRunWorldHandler)
		{
			Task chunkTask;
			for (int max = 25; max > 0 && taskManager.PopTask(Task::RequestChunk, chunkTask); max--)
			{
				if (!RequestChunk(chunkTask.pos))
					taskManager.PushTask(chunkTask);
			}
		}


	}
}

void PlaceBot::MessageHandler(const std::shared_ptr<Protocol::IS2CMessage>& message)
{
	ConnectionBot::MessageHandler(message);

	if (mRunWorldHandler)
		mManager.GetWorld().HandlePacket(message);
}

