#pragma once
#define NOMINMAX
#include <Windows.h>
#include <SFML/Graphics.hpp>

namespace ScreenCaptureHelper
{
	sf::Vector2i getWindowSize(HWND hwnd);
	sf::Image hwndToImage(HWND hwnd, const sf::Vector2i imageSize);
	
}