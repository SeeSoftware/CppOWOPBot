#pragma once
#include <cstdint>
#include <SFML/Graphics.hpp>

namespace OWOP
{
	using CursorID = uint32_t;
	using ToolID = uint8_t;
	using RankID = uint8_t;

	const sf::Color NULL_COLOR = sf::Color(0, 0, 0, 0);
	constexpr size_t MAX_WORLDNAME_LENGTH = 24;
	constexpr int CHUNK_SIZE = 16;
	constexpr uint16_t WORLD_VERIFICATION = 4321; //4321,1234
	constexpr char CHAT_VERIFICATION[] = "\x0A";
	constexpr char TOKEN_VERIFICATION[] = "CaptchA";


	enum class CaptchaState : uint8_t
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
		

		operator sf::Color() const
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
		CursorData(CursorID id, const sf::Vector2i &subPos,const Color &col = Color(255, 255, 255), ToolID tool = 0) : id(id), subPos(subPos), color(col), tool(tool) {};

		sf::Vector2i GetWorldPos() const
		{
			return subPos / 16;
		}
		void SetWorldPos(const sf::Vector2i & worldPos)
		{
			subPos = worldPos * 16;
		}

		CursorID id;
		sf::Vector2i subPos; //in pixel * 16 positions
		Color color;
		OWOP::ToolID tool;
	};


	struct TileUpdate
	{
		TileUpdate() = default;

		CursorID cursorId;
		sf::Vector2i worldPos;
		Color pxCol;
	};

#pragma pack(pop)

}