#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include "Bots/BotManager.h"

class UInterface;

class ITool
{
public:

	ITool(UInterface &ui,BotManager &manager, sf::RenderWindow &target) : mUi(ui),mManager(manager), mTarget(target) {};
	virtual ~ITool() = default;

	virtual void DrawGui() = 0;
	virtual void ProcessEvent(const sf::Event &e) = 0;
	//focused is if background is focused
	virtual void Update(float dt) = 0;
	
	virtual std::string ToolName() const = 0;
	virtual sf::Texture &ToolImage() const = 0;

protected:

	sf::RenderWindow &mTarget;
	BotManager &mManager;
	UInterface &mUi;
};