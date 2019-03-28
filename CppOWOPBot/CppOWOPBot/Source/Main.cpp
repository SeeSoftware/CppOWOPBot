#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <iostream>

#include <SFML/Graphics.hpp>
#include "IncImgui.h"

#include "Bots/BotManager.h"

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	sf::Clock frameClock;
	sf::RenderWindow window;
	window.create(sf::VideoMode(1280, 720), "OWOP Bot");

	ImGui::SFML::Init(window);
	

	BotManager manager;
	manager.Connect("wss://ourworldofpixels.com");

	while (window.isOpen())
	{
		sf::Event e;
		while (window.pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
				window.close();

			ImGui::SFML::ProcessEvent(e);
		}

		ImGui::SFML::Update(window, frameClock.restart());
		manager.Poll();
		

		window.clear();

		ImGui::Begin("Test");
		ImGui::End();
		
		ImGui::DrawRect(sf::FloatRect(40, 40, 100, 100), sf::Color(255, 0, 0));

		ImGui::SFML::Render(window);
		window.display();


	}

	ImGui::SFML::Shutdown();

	return 0;
}