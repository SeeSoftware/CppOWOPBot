#include "ShapeTool.h"

void ShapeTool::DrawGui()
{
}

void ShapeTool::ProcessEvent(const sf::Event & e)
{
}

void ShapeTool::Update(float dt)
{
}

std::string ShapeTool::ToolName() const
{
	return "Shape Tool";
}

sf::Texture ShapeTool::ToolImage() const
{
	static sf::Texture texture;
	static bool loaded = false;
	if (!loaded)
		texture.loadFromFile("Images/ShapeTool.png"), loaded = true;

	return texture;
}
