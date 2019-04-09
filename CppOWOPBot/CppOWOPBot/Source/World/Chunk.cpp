#include "Chunk.h"
#include "SFExtraMath.h"
#include <iostream>

Chunk::Chunk()
{
	mTexture.create(OWOP::CHUNK_SIZE, OWOP::CHUNK_SIZE);
	mTextureData.resize(OWOP::CHUNK_SIZE*OWOP::CHUNK_SIZE,OWOP::NULL_COLOR);
	UpdateTexture();
}

Chunk::Chunk(const sf::Vector2i & chunkPos, bool locked)
	:mChunkPos(chunkPos), mLocked(locked)
{
	mTexture.create(OWOP::CHUNK_SIZE, OWOP::CHUNK_SIZE);
	mTextureData.resize(OWOP::CHUNK_SIZE*OWOP::CHUNK_SIZE, OWOP::NULL_COLOR);
	UpdateTexture();
}

void Chunk::SetPixel(const sf::Vector2i & localChunkPos, sf::Color color)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);

	if (VectorToIndex(localChunkPos) >= mTextureData.size())
		return;

	color.a = 255;
	mTextureData[VectorToIndex(localChunkPos)] = color;
	mDirty = true;
}

sf::Color Chunk::GetPixel(const sf::Vector2i & localChunkPos) const
{
	std::shared_lock<std::shared_mutex> lock(mMutex);

	if (VectorToIndex(localChunkPos) >= mTextureData.size())
		return OWOP::NULL_COLOR;

	return mTextureData[VectorToIndex(localChunkPos)];
}

sf::Vector2i Chunk::LocalToWorldPos(const sf::Vector2i & localChunkPos) const
{
	return localChunkPos + mChunkPos * sf::Vector2i(OWOP::CHUNK_SIZE, OWOP::CHUNK_SIZE);
}

sf::Vector2i Chunk::WorldToLocalPos(const sf::Vector2i & worldPos) const
{
	return worldPos - mChunkPos * sf::Vector2i(OWOP::CHUNK_SIZE, OWOP::CHUNK_SIZE);
}

void Chunk::Draw(sf::RenderTarget & target) const
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	if (mDirty)
	{
		UpdateTexture();
		mDirty = false;
	}

	static sf::RectangleShape shape(sf::Vector2f(OWOP::CHUNK_SIZE, OWOP::CHUNK_SIZE));
	shape.setPosition(sf::Vector2f(OWOP::CHUNK_SIZE, OWOP::CHUNK_SIZE) * mChunkPos);
	shape.setFillColor(sf::Color(255, 255, 255));
	shape.setTexture(&mTexture);
	
	target.draw(shape);
}

void Chunk::LoadFromChunkData(const std::vector<OWOP::Color>& data)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	
	if (data.size() > mTextureData.size())
	{
		std::cout << "LoadFromChunkData: Data is too big, aborting load.\n";
		return;
	}

	for (int i = 0; i < data.size(); i++)
	{
		//TODO: optimize with memcpy
		mTextureData[i].r = data[i].r;
		mTextureData[i].g = data[i].g;
		mTextureData[i].b = data[i].b;
		mTextureData[i].a = 255;
	}

	mDirty = true;
}
