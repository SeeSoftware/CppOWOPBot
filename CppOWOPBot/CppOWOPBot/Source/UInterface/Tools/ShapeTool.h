#pragma once
#include "ITool.h"

class ShapeTool : public ITool
{
public:

	ShapeTool(UInterface &ui, BotManager &manager, sf::RenderWindow &target) : ITool(ui,manager,target) {}

	// Geerbt über ITool
	virtual void DrawGui() override;
	virtual void ProcessEvent(const sf::Event & e) override;
	virtual void Update(float dt) override;

	virtual std::string ToolName() const override;
	virtual sf::Texture &ToolImage() const override;
};