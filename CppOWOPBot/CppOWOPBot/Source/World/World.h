#pragma once
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <memory>

#include <SFML/Graphics.hpp>

#include "OWOP/Structs.h"
#include "OWOP/Protocol.h"
#include "Chunk.h"
#include "SFExtraMath.h"

//access is threadsafe, but might be not atomic
class World
{
public:
	
	World();

	//access is threadsafe
	void HandlePacket(const std::shared_ptr<Protocol::IS2CMessage> &message);

	//access is threadsafe
	sf::Color GetPixel(const sf::Vector2i &worldPos) const;
	//Note: this doesn't send a UpdatePixel to the server
	//access is threadsafe
	void SetPixel(const sf::Vector2i &worldPos, const sf::Color &color);


	void Draw(sf::RenderTarget &target) const;

	//access is threadsafe
	void ClearWorld();

	//access is threadsafe
	bool ChunkExists(const sf::Vector2i &chunkPos) const;

	//access is threadsafe
	bool IsChunkLocked(const sf::Vector2i &chunkPos) const;

	//for deleting/preloading chunks
	void DeleteChunk(const sf::Vector2i &chunkPos);
	//creates chunk if doesnt exist
	void CreateChunk(const sf::Vector2i &chunkPos);

	sf::Vector2i WorldToChunk(const sf::Vector2i &worldPos) const;

	//access is threadsafe
	bool GetCursorData(OWOP::CursorID cursorID, OWOP::CursorData &data) const
	{
		std::shared_lock<std::shared_mutex> lock(mMutex);
		auto iter = mCursors.find(cursorID);
		if (iter != mCursors.end())
		{
			data = (*iter).second;
			return true;
		}
		return false;
	}

	//access is threadsafe
	std::unordered_map<OWOP::CursorID, OWOP::CursorData> GetAllCursors() const
	{
		std::shared_lock<std::shared_mutex> lock(mMutex);
		return mCursors;
	}


private:

	//will create a chunk if it doest exist
	//not thread safe
	std::shared_ptr<Chunk> GetChunk(const sf::Vector2i &chunkPos);

	//not thread safe
	void SetPixelUnsafe(const sf::Vector2i &worldPos, const sf::Color &color);

	mutable std::shared_mutex mMutex;
	std::unordered_map<sf::Vector2i, std::shared_ptr<Chunk>> mChunks;
	std::unordered_map<OWOP::CursorID, OWOP::CursorData> mCursors;

	sf::Texture mToolsTexture;

};