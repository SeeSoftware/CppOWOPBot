#include "World.h"
#include "Util.h"

World::World()
{
	mToolsTexture.loadFromFile("Images/toolset.png");
}

void World::HandlePacket(const std::shared_ptr<Protocol::IS2CMessage>& message)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);

	switch (message->opcode)
	{
		case Protocol::PacketOpCode::WorldUpdate:
		{
			std::shared_ptr<Protocol::WorldUpdate> castMsg = Protocol::DowncastMessage<Protocol::WorldUpdate>(message);
			
			for (OWOP::CursorData &cd : castMsg->cursorUpdates)
				mCursors[cd.id] = cd;

			for (OWOP::TileUpdate &tu : castMsg->tileUpdates)
				SetPixelUnsafe(tu.worldPos, tu.pxCol);

			for (OWOP::CursorID &di : castMsg->disconnects)
				mCursors.erase(di);

			break;
		}

		case Protocol::PacketOpCode::ChunkLoad:
		{
			std::shared_ptr < Protocol::ChunkData > castMsg = Protocol::DowncastMessage<Protocol::ChunkData>(message);

			std::shared_ptr<Chunk> chunk = GetChunk(castMsg->chunkPos);
			chunk->LoadFromChunkData(castMsg->Decompress());
			chunk->SetLocked(castMsg->locked);
			break;
		}

		case Protocol::PacketOpCode::ChunkProtected:
		{
			std::shared_ptr < Protocol::ChunkProtected > castMsg = Protocol::DowncastMessage<Protocol::ChunkProtected>(message);

			std::shared_ptr<Chunk> chunk = GetChunk(castMsg->chunkPos);
			chunk->SetLocked(castMsg->newState);
			break;
		}


		default:
			break;
	}
}

sf::Color World::GetPixel(const sf::Vector2i & worldPos) const
{
	std::shared_lock<std::shared_mutex> lock(mMutex);

	sf::Vector2i chunkPos = WorldToChunk(worldPos);
	if (mChunks.count(chunkPos) == 0)
		return OWOP::NULL_COLOR;

	std::shared_ptr<Chunk> chunk = mChunks.at(chunkPos);
	return chunk->GetPixel(chunk->WorldToLocalPos(worldPos));
}

void World::SetPixel(const sf::Vector2i & worldPos, const sf::Color & color)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	SetPixelUnsafe(worldPos, color);
}

void World::Draw(sf::RenderTarget & target) const
{
	std::shared_lock<std::shared_mutex> lock(mMutex);

	sf::Vector2i topleft((int)((-target.getView().getSize().x / 2.0f) + target.getView().getCenter().x), (int)((-target.getView().getSize().y / 2.0f) + target.getView().getCenter().y));
	sf::Vector2i bottomright((int)((target.getView().getSize().x / 2.0f) + target.getView().getCenter().x), (int)((target.getView().getSize().y / 2.0f) + target.getView().getCenter().y));

	sf::IntRect scrRect(topleft - sf::Vector2i(OWOP::CHUNK_SIZE, OWOP::CHUNK_SIZE), bottomright - (topleft - sf::Vector2i(OWOP::CHUNK_SIZE, OWOP::CHUNK_SIZE)));

	for (const auto &x : mChunks)
	{
		if (scrRect.contains(x.second->GetChunkPos() * sf::Vector2i(OWOP::CHUNK_SIZE,OWOP::CHUNK_SIZE)))
		{
			x.second->Draw(target);
		}
	}

	for (const auto &x : mCursors)
	{
		static sf::RectangleShape CursorShape(sf::Vector2f(32,32));
		CursorShape.setTexture(&mToolsTexture);
		CursorShape.setTextureRect(sf::IntRect(0, 0, 32, 32));
		CursorShape.setScale(sf::Vector2f(0.1f, 0.1f));
		CursorShape.setFillColor(sf::Color(255, 255, 255));
		CursorShape.setPosition((float)x.second.GetWorldPos().x, (float)x.second.GetWorldPos().y); //((float)x.second.subPos.x/16.0f, (float)x.second.subPos.y / 16.0f);
		target.draw(CursorShape);
	}
}

void World::ClearWorld()
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	mChunks.clear();
	mCursors.clear();
}

bool World::ChunkExists(const sf::Vector2i & chunkPos) const
{
	std::shared_lock<std::shared_mutex> lock(mMutex);
	return mChunks.count(chunkPos) > 0;
}

bool World::IsChunkLocked(const sf::Vector2i & chunkPos) const
{
	std::shared_lock<std::shared_mutex> lock(mMutex);
	if (mChunks.count(chunkPos) == 0)
		return true;
	return mChunks.at(chunkPos)->IsLocked();
}

void World::DeleteChunk(const sf::Vector2i & chunkPos)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	mChunks.erase(chunkPos);
}

void World::CreateChunk(const sf::Vector2i & chunkPos)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	GetChunk(chunkPos);
}

sf::Vector2i World::WorldToChunk(const sf::Vector2i & worldPos) const
{
	return sf::Vector2i(Util::div_floor(worldPos.x, OWOP::CHUNK_SIZE), Util::div_floor(worldPos.y, OWOP::CHUNK_SIZE));
}

std::shared_ptr<Chunk> World::GetChunk(const sf::Vector2i & chunkPos)
{
	if (mChunks.count(chunkPos) == 0)
		mChunks[chunkPos] = std::make_shared<Chunk>(chunkPos);
	return mChunks[chunkPos];
}

void World::SetPixelUnsafe(const sf::Vector2i & worldPos, const sf::Color & color)
{
	sf::Vector2i chunkPos = WorldToChunk(worldPos);

	if (mChunks.count(chunkPos) == 0)
		return;

	std::shared_ptr<Chunk> chunk = GetChunk(chunkPos);
	chunk->SetPixel(chunk->WorldToLocalPos(worldPos), color);
}
