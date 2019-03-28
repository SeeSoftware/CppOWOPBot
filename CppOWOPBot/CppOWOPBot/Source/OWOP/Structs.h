#pragma once
#include <cstdint>
#include <SFML/Graphics.hpp>

namespace OWOP
{
	typedef uint32_t CursorID;
	typedef uint8_t ToolID;

	constexpr int CHUNK_SIZE = 16;
	constexpr uint16_t WORLD_VERIFICATION = 4321; //4321,1234
	constexpr char CHAT_VERIFICATION[] = "\x0A";
	constexpr char TOKEN_VERIFICATION[] = "CaptchA";

	enum PacketOpCode : uint8_t
	{
		SetID,
		WorldUpdate,
		ChunkLoad,
		Teleport,
		SetRank,
		Captcha,
		SetPQuota,
		ChunkProtected
	};

	enum CaptchaState : uint8_t
	{
		CA_WAITING,
		CA_VERIFYING,
		CA_VERIFIED,
		CA_OK,
		CA_INVALID
	};


#pragma pack(push,1)

	struct Color
	{
		Color() = default;
		Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b){}
		Color(sf::Color col) : r(col.r),g(col.g),b(col.b){}
		

		operator sf::Color()
		{
			return sf::Color(r, g, b);
		}

		uint8_t r;
		uint8_t g;
		uint8_t b;
	};

	struct CursorData
	{
		CursorData() = default;
		CursorData(CursorID id, const sf::Vector2i &subPos, Color col = Color(255, 255, 255), ToolID tool = 0) : id(id), subPos(subPos), color(col), tool(tool) {};

		CursorID id;
		sf::Vector2i subPos; //in pixel * 16 positions
		Color color;
		ToolID tool;
	};


	struct TileUpdate
	{
		TileUpdate() = default;

		CursorID id;
		sf::Vector2i pos;
		Color pxCol;
	};

#pragma pack(pop)

}