#include "World.h"
#include "Util.h"

UnsafeWorld::UnsafeWorld()
{
	mToolsTexture.loadFromFile("Images/toolset.png");
}

void UnsafeWorld::HandlePacket(const std::shared_ptr<Protocol::IS2CMessage>& message)
{
	switch (message->opcode)
	{
		case Protocol::PacketOpCode::WorldUpdate:
		{
			std::shared_ptr<Protocol::WorldUpdate> castMsg = Protocol::DowncastMessage<Protocol::WorldUpdate>(message);
			
			for (OWOP::CursorData &cd : castMsg->cursorUpdates)
				mCursors[cd.id] = cd;

			for (OWOP::TileUpdate &tu : castMsg->tileUpdates)
			{
				SetPixel(tu.worldPos, tu.pxCol);
				SetPixel(tu.worldPos, tu.pxCol, Chunk::BufferType::Back);
			}

			for (OWOP::CursorID &di : castMsg->disconnects)
				mCursors.erase(di);

			break;
		}

		case Protocol::PacketOpCode::ChunkLoad:
		{
			std::shared_ptr < Protocol::ChunkData > castMsg = Protocol::DowncastMessage<Protocol::ChunkData>(message);

			std::shared_ptr<Chunk> chunk = GetChunk(castMsg->chunkPos);

			if (!chunk)
			{
				std::cout << "What the fuck!\n";
				abort(); //abort before you die!
			}

			chunk->LoadFromChunkData(castMsg->Decompress());
			chunk->SetLocked(castMsg->locked);
			break;
		}

		case Protocol::PacketOpCode::ChunkProtected:
		{
			std::shared_ptr < Protocol::ChunkProtected > castMsg = Protocol::DowncastMessage<Protocol::ChunkProtected>(message);

			std::shared_ptr<Chunk> chunk = GetChunk(castMsg->chunkPos);

			if (!chunk)
			{
				std::cout << "What the fuck!\n";
				abort(); //abort before you die!
			}

			chunk->SetLocked(castMsg->newState);
			break;
		}


		default:
			break;
	}
}

sf::Color UnsafeWorld::GetPixel(const sf::Vector2i & worldPos, Chunk::BufferType btype) const
{

	sf::Vector2i chunkPos = WorldToChunk(worldPos);
	if (mChunks.count(chunkPos) == 0)
		return OWOP::NULL_COLOR;

	std::shared_ptr<Chunk> chunk = mChunks.at(chunkPos);
	return chunk->GetPixel(chunk->WorldToLocalPos(worldPos), btype);
}

void UnsafeWorld::SetPixel(const sf::Vector2i & worldPos, const sf::Color & color, Chunk::BufferType btype)
{
	sf::Vector2i chunkPos = WorldToChunk(worldPos);

	if (mChunks.count(chunkPos) == 0)
		return;

	std::shared_ptr<Chunk> chunk = GetChunk(chunkPos);

	if (!chunk)
	{
		std::cout << "What the fuck!\n";
		abort(); //abort before you die!
	}

	chunk->SetPixel(chunk->WorldToLocalPos(worldPos), color, btype);
}

void UnsafeWorld::Draw(sf::RenderTarget & target) const
{

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

void UnsafeWorld::ClearWorld()
{
	mChunks.clear();
	mCursors.clear();
}

bool UnsafeWorld::ChunkExists(const sf::Vector2i & chunkPos) const
{
	return mChunks.count(chunkPos) > 0;
}

bool UnsafeWorld::IsChunkLocked(const sf::Vector2i & chunkPos) const
{
	if (mChunks.count(chunkPos) == 0)
		return true;
	return mChunks.at(chunkPos)->IsLocked();
}

void UnsafeWorld::DeleteChunk(const sf::Vector2i & chunkPos)
{
	mChunks.erase(chunkPos);
}

void UnsafeWorld::CreateChunk(const sf::Vector2i & chunkPos)
{
	GetChunk(chunkPos);
}

sf::Vector2i UnsafeWorld::WorldToChunk(const sf::Vector2i & worldPos) const
{
	return sf::Vector2i(Util::div_floor(worldPos.x, OWOP::CHUNK_SIZE), Util::div_floor(worldPos.y, OWOP::CHUNK_SIZE));
}

std::shared_ptr<Chunk> UnsafeWorld::GetChunk(const sf::Vector2i & chunkPos)
{
	if (mChunks.count(chunkPos) == 0)
		mChunks[chunkPos] = std::make_shared<Chunk>(chunkPos);
	return mChunks.at(chunkPos);
}






void World::HandlePacket(const std::shared_ptr<Protocol::IS2CMessage>& message)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	mWorld.HandlePacket(message);
}

sf::Color World::GetPixel(const sf::Vector2i & worldPos, Chunk::BufferType btype) const
{
	std::shared_lock<std::shared_mutex> lock(mMutex);
	return mWorld.GetPixel(worldPos, btype);
}

void World::SetPixel(const sf::Vector2i & worldPos, const sf::Color & color, Chunk::BufferType btype)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	mWorld.SetPixel(worldPos, color, btype);
}

void World::Draw(sf::RenderTarget & target) const
{
	std::shared_lock<std::shared_mutex> lock(mMutex);
	mWorld.Draw(target);
}

void World::ClearWorld()
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	mWorld.ClearWorld();
}

bool World::ChunkExists(const sf::Vector2i & chunkPos) const
{
	std::shared_lock<std::shared_mutex> lock(mMutex);
	return mWorld.ChunkExists(chunkPos);
}

bool World::IsChunkLocked(const sf::Vector2i & chunkPos) const
{
	std::shared_lock<std::shared_mutex> lock(mMutex);
	return mWorld.IsChunkLocked(chunkPos);
}

void World::DeleteChunk(const sf::Vector2i & chunkPos)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	return mWorld.DeleteChunk(chunkPos);
}

void World::CreateChunk(const sf::Vector2i & chunkPos)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	return mWorld.CreateChunk(chunkPos);
}

sf::Vector2i World::WorldToChunk(const sf::Vector2i & worldPos) const
{
	return mWorld.WorldToChunk(worldPos);
}

void World::Update(std::function<void(UnsafeWorld&)> callback)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	callback(mWorld);
}
