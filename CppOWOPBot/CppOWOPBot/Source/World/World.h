#pragma once
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <functional>

#include <SFML/Graphics.hpp>

#include "OWOP/Structs.h"
#include "OWOP/Protocol.h"
#include "Chunk.h"
#include "SFExtraMath.h"


class UnsafeWorld
{
public:
	
	UnsafeWorld();

	void HandlePacket(const std::shared_ptr<Protocol::IS2CMessage> &message);

	sf::Color GetPixel(const sf::Vector2i &worldPos, Chunk::BufferType btype = Chunk::BufferType::Front) const;
	void SetPixel(const sf::Vector2i &worldPos, const sf::Color &color, Chunk::BufferType btype = Chunk::BufferType::Front);


	void Draw(sf::RenderTarget &target) const;

	void ClearWorld();

	//creates chunk if it doesnt exist
	std::shared_ptr<Chunk> GetChunk(const sf::Vector2i &chunkPos);
	bool ChunkExists(const sf::Vector2i &chunkPos) const;
	bool IsChunkLocked(const sf::Vector2i &chunkPos) const;
	void DeleteChunk(const sf::Vector2i &chunkPos);
	void CreateChunk(const sf::Vector2i &chunkPos);

	sf::Vector2i WorldToChunk(const sf::Vector2i &worldPos) const;

	std::unordered_map<sf::Vector2i, std::shared_ptr<Chunk>> &GetChunks() { return mChunks; }
	std::unordered_map<OWOP::CursorID, OWOP::CursorData> &GetCursors() { return mCursors; }

	const std::unordered_map<sf::Vector2i, std::shared_ptr<Chunk>> &GetChunks() const { return mChunks; }
	const std::unordered_map<OWOP::CursorID, OWOP::CursorData> &GetCursors() const { return mCursors; }


private:

	std::unordered_map<sf::Vector2i, std::shared_ptr<Chunk>> mChunks;
	std::unordered_map<OWOP::CursorID, OWOP::CursorData> mCursors;

	sf::Texture mToolsTexture;

};

//access is threadsafe, but might be not atomic
class World
{
public:

	//access is threadsafe
	void HandlePacket(const std::shared_ptr<Protocol::IS2CMessage> &message);

	//access is threadsafe
	sf::Color GetPixel(const sf::Vector2i &worldPos, Chunk::BufferType btype = Chunk::BufferType::Front) const;
	//Note: this doesn't send a UpdatePixel to the server
	//access is threadsafe
	void SetPixel(const sf::Vector2i &worldPos, const sf::Color &color, Chunk::BufferType btype = Chunk::BufferType::Front);


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

	void Update(std::function<void(UnsafeWorld &)> callback);

	bool GetCursorData(OWOP::CursorID cursorID, OWOP::CursorData &data) const
	{
		std::unique_lock<std::recursive_mutex> lock(mMutex);
		auto iter = mWorld.GetCursors().find(cursorID);
		if (iter != mWorld.GetCursors().end())
		{
			data = (*iter).second;
			return true;
		}
		return false;
	}

	//access is threadsafe
	std::unordered_map<OWOP::CursorID, OWOP::CursorData> GetAllCursors() const
	{
		std::unique_lock<std::recursive_mutex> lock(mMutex);
		return mWorld.GetCursors();
	}

private:

	UnsafeWorld mWorld;
	mutable std::recursive_mutex mMutex; //have to use this because of the update function to prevent double locking but i loose the advantages of a shared_mutex :(
};
