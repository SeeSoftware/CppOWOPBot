#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include "Bots/BotManager.h"

class ITool
{
public:

	ITool(BotManager &manager, sf::RenderWindow &target) : mManager(manager), mTarget(target) {};
	virtual ~ITool() = default;

	virtual void DrawGui() = 0;
	virtual void ProcessEvent(const sf::Event &e) = 0;
	virtual void Update(float dt) = 0;
	
	virtual std::string ToolName() const = 0;
	virtual sf::Texture ToolImage() const = 0;

protected:

	sf::RenderWindow &mTarget;
	BotManager &mManager;
};