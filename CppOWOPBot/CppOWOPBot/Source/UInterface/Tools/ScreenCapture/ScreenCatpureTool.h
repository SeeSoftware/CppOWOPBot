#pragma once
#include <string>
#include <SFML/Graphics.hpp>

#include "../ITool.h"

class ScreenCaptureTool : public ITool
{
public:

	ScreenCaptureTool(UInterface& ui, BotManager& manager, sf::RenderWindow& target);

	// Geerbt über ITool
	virtual void DrawGui() override;
	virtual void ProcessEvent(const sf::Event& e) override;
	virtual void Update(float dt) override;

	virtual std::string ToolName() const override;
	virtual sf::Texture& ToolImage() const override;

private:

	sf::Image CaptureImage();
	void SortPixelTasks();

	bool mIsEnabled = false;
	bool mReduceColors = true;
	bool mDitherColors = true;

	HWND mHWND = 0;
	sf::Vector2i mImagePos = sf::Vector2i(0,0);
	sf::Vector2i mImageSize;

	sf::Clock mPreviewClock;
};