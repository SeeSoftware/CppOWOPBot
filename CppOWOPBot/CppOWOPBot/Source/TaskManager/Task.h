#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "OWOP/Structs.h"

struct Task
{
	
	enum Type
	{
		None,
		PlacePixel,
		RequestChunk,
		SendChat
	};

	Task() = default;
	Task(Type type, const sf::Vector2i &pos = sf::Vector2i(0, 0), const sf::Color &col = OWOP::NULL_COLOR) : type(type),pos(pos), color(col) {}
	Task(Type type, const std::string &message = "") : type(type),message(message){}

	Type type = Type::None;
	
	sf::Vector2i pos = sf::Vector2i(0,0);
	sf::Color color = OWOP::NULL_COLOR;
	std::string message = "";

};