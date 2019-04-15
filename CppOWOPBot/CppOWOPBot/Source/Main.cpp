#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <iostream>

#include <SFML/Graphics.hpp>
#include "Imgui/IncImgui.h"

#include "UInterface/UInterface.h"
#include "Bots/BotManager.h"

#include "UInterface/Tools/ShapeTool.h"

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	sf::Clock frameClock;
	sf::RenderWindow window;

	sf::ContextSettings settings;
	settings.antialiasingLevel = 4;
	window.create(sf::VideoMode(1600, 900), "OWOP Bot",7U, settings);

	ImGui::SFML::Init(window);


	BotManager manager;
	manager.GetProxyList().LoadProxyfile("proxylist.txt");
	manager.GetProxyList().SetSettings(5);
	manager.Connect("wss://ourworldofpixels.com", 1, false);

	UInterface ui(manager,window);
	ui.AddTool<ShapeTool>(); 
	ui.AddTool<ShapeTool>();
	ui.AddTool<ShapeTool>();
	
	while (window.isOpen())
	{
		sf::Event e;
		while (window.pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
				window.close();

			ImGui::SFML::ProcessEvent(e);
			ui.ProcessEvent(e);
			
		}

		////////////////////////////////////////////////////////////////////////
		sf::Vector2i topleft((int)((-window.getView().getSize().x / 2.0f) + window.getView().getCenter().x), (int)((-window.getView().getSize().y / 2.0f) + window.getView().getCenter().y));
		sf::Vector2i bottomright((int)((window.getView().getSize().x / 2.0f) + window.getView().getCenter().x), (int)((window.getView().getSize().y / 2.0f) + window.getView().getCenter().y));

		sf::IntRect scrRect(topleft - sf::Vector2i(OWOP::CHUNK_SIZE, OWOP::CHUNK_SIZE), bottomright - (topleft - sf::Vector2i(OWOP::CHUNK_SIZE, OWOP::CHUNK_SIZE)));
		
		for(int y = scrRect.top / OWOP::CHUNK_SIZE; y < (scrRect.top + scrRect.height) / OWOP::CHUNK_SIZE; y++)
			for (int x = scrRect.left / OWOP::CHUNK_SIZE; x < (scrRect.left + scrRect.width) / OWOP::CHUNK_SIZE; x++)
			{
				sf::Vector2i chunkPos = sf::Vector2i(x, y);
				if (!manager.GetWorld().ChunkExists(chunkPos))
				{
					manager.GetWorld().CreateChunk(chunkPos);
					manager.GetTaskManager().PushTask(Task(Task::RequestChunk, chunkPos));
				}
			}
		////////////////////////////////////////////////////////////////////////

		/////////////////
		sf::Time frameTime = frameClock.restart();

		ImGui::SFML::Update(window, frameTime);
		manager.Update(frameTime.asSeconds());

		ui.Update(frameTime.asSeconds());
		////////////////////////


		window.clear();
		manager.Draw(window);
		ui.DrawGui();

		ImGui::SFML::Render(window);
		window.display();


	}

	ImGui::SFML::Shutdown();
	
	return 0;

}