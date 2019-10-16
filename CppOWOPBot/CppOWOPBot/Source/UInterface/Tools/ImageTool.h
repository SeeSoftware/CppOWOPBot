#pragma once
#include <string>
#include <SFML/Graphics.hpp>

#include "ITool.h"

class ImageTool : public ITool
{
public:

	ImageTool(UInterface &ui, BotManager &manager, sf::RenderWindow &target) : ITool(ui,manager, target) {}

	// Geerbt über ITool
	virtual void DrawGui() override;
	virtual void ProcessEvent(const sf::Event & e) override;
	virtual void Update(float dt) override;

	virtual std::string ToolName() const override;
	virtual sf::Texture &ToolImage() const override;

private:

	void SortPixelTasks();
	
	sf::Texture mPreview;
	sf::Image mImage;
	std::string mFilename = "Images/image.png";
};