#include "Chunk.h"
#include "SFExtraMath.h"
#include <iostream>

Chunk::Chunk()
{
	mTexture.create(OWOP::CHUNK_SIZE, OWOP::CHUNK_SIZE);
	mFrontBuffer.resize(OWOP::CHUNK_SIZE*OWOP::CHUNK_SIZE,OWOP::NULL_COLOR);
	mBackBuffer.resize(OWOP::CHUNK_SIZE*OWOP::CHUNK_SIZE, OWOP::NULL_COLOR);
	UpdateTexture();
}

Chunk::Chunk(const sf::Vector2i & chunkPos, bool locked)
	:mChunkPos(chunkPos), mLocked(locked)
{
	mTexture.create(OWOP::CHUNK_SIZE, OWOP::CHUNK_SIZE);
	mFrontBuffer.resize(OWOP::CHUNK_SIZE*OWOP::CHUNK_SIZE, OWOP::NULL_COLOR);
	mBackBuffer.resize(OWOP::CHUNK_SIZE*OWOP::CHUNK_SIZE, OWOP::NULL_COLOR);
	UpdateTexture();
}

void Chunk::SetPixel(const sf::Vector2i & localChunkPos, sf::Color color, BufferType btype)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);

	BufferVec &buffer = SelectBuffer(btype);

	size_t index = VectorToIndex(localChunkPos);
	if (index >= buffer.size())
		return;

	color.a = 255;
	buffer[index] = color;

	if(btype == BufferType::Front)
		mDirty = true;
}

sf::Color Chunk::GetPixel(const sf::Vector2i & localChunkPos, BufferType btype) const
{
	std::shared_lock<std::shared_mutex> lock(mMutex);

	const BufferVec &buffer = SelectBuffer(btype);

	size_t index = VectorToIndex(localChunkPos);
	if (index >= buffer.size())
		return OWOP::NULL_COLOR;

	return buffer[index];
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
	
	if (data.size() > mFrontBuffer.size() || data.size() > mBackBuffer.size())
	{
		std::cout << "LoadFromChunkData: Data is too big, aborting load.\n";
		return;
	}

	for (int i = 0; i < data.size(); i++)
	{
		//TODO: optimize with memcpy
		mFrontBuffer[i].r = data[i].r;
		mFrontBuffer[i].g = data[i].g;
		mFrontBuffer[i].b = data[i].b;
		mFrontBuffer[i].a = 255;
	}

	mBackBuffer = mFrontBuffer;

	mDirty = true;
}
