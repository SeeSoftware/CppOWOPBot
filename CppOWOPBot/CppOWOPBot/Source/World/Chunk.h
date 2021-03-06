#pragma once
#include <SFML/Graphics.hpp>
#include <stdint.h>
#include <shared_mutex>
#include <mutex>
#include <atomic>
#include "OWOP/Structs.h"

//ALL MEMBERS ARE THREAD SAFE AND ATOMIC !
class Chunk
{
public:

	enum class BufferType
	{
		Front,
		Back
	};

	Chunk();
	Chunk(const sf::Vector2i &chunkPos, bool locked = false);

	//access is threadsafe with unique_lock
	void SetPixel(const sf::Vector2i &localChunkPos, sf::Color color, BufferType btype = BufferType::Front);
	//access is threadsafe with shared_lock
	sf::Color GetPixel(const sf::Vector2i &localChunkPos, BufferType btype = BufferType::Front) const;

	sf::Vector2i LocalToWorldPos(const sf::Vector2i &localChunkPos) const;
	sf::Vector2i WorldToLocalPos(const sf::Vector2i &worldPos) const;

	//access is threadsafe with unique_lock
	void Draw(sf::RenderTarget &target) const;
	//access is threadsafe with unique_lock
	void LoadFromChunkData(const std::vector<OWOP::Color> &data);

	
	void SetChunkPos(const sf::Vector2i &chunkPos)
	{
		std::unique_lock<std::shared_mutex> lock(mMutex);
		mChunkPos = chunkPos;
	}
	
	sf::Vector2i GetChunkPos() const
	{
		std::shared_lock<std::shared_mutex> lock(mMutex);
		return mChunkPos;
	}

	void SetLocked(bool locked)
	{
		mLocked = locked;
	}

	bool IsLocked() const
	{
		return mLocked;
	}

	bool IsDirty() const
	{
		return mDirty;
	}


private:

	using BufferVec = std::vector<sf::Color>;

	inline size_t VectorToIndex(const sf::Vector2i &localChunkPos) const
	{
		return localChunkPos.x + localChunkPos.y * OWOP::CHUNK_SIZE;
	}
	inline void UpdateTexture() const
	{
		mTexture.update((sf::Uint8*)mFrontBuffer.data());
	}

	inline BufferVec &SelectBuffer(BufferType buf)
	{
		return buf == BufferType::Front ? mFrontBuffer : mBackBuffer;
	}

	inline const BufferVec &SelectBuffer(BufferType buf) const
	{
		return buf == BufferType::Front ? mFrontBuffer : mBackBuffer;
	}


	mutable std::shared_mutex mMutex;
	std::atomic<bool> mLocked = true;
	sf::Vector2i mChunkPos;


	BufferVec mFrontBuffer;
	BufferVec mBackBuffer;
	mutable sf::Texture mTexture;
	mutable std::atomic<bool> mDirty;
};