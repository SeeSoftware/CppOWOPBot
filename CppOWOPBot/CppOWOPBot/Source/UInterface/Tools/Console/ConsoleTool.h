#pragma once
#include "../ITool.h"
#include <SFML/Graphics.hpp>
#include <vector>


class ConsoleTool : public ITool
{
public:

	ConsoleTool(UInterface &ui, BotManager &manager, sf::RenderWindow &target);


	void SubmitCommand(const std::string &text);

	//position is the vertical position of the text
	void ConsoleSetPos(const sf::Vector2i &pos);
	void ConsolePutc(char c);
	void ConsolePuts(const std::string &text);
	void ConsoleShiftUp(int ammount = 1);
	void ClearConsole();

	// Geerbt über ITool
	virtual void DrawGui() override;
	virtual void ProcessEvent(const sf::Event & e) override;
	virtual void Update(float dt) override;

	virtual std::string ToolName() const override;
	virtual sf::Texture &ToolImage() const override;

private:
	//resizes buffer and texture to necessary size
	//but clears the console
	void UpdateResolution();

	void UpdateAndDrawConsole(float dt);

	//attaches cmd.exe
	void AttachCMD();
	//replaces text buffer with cmdtext
	void UpdateTextBufferFromCMD();

	std::vector<char> mTextBuffer;
	sf::Vector2i mCurrentTextPos = sf::Vector2i(0,0);
	sf::Vector2i mResolution = sf::Vector2i(120, 30);
	
	sf::Vector2i mPosition = sf::Vector2i(496,496);
	sf::Vector2i mFontSpacing = sf::Vector2i(1, 1);

	static const constexpr int FONT_SIZE = 8;

	std::string mCurrentCommand = "";
	bool mIsEnabled = false;



	sf::Font mFont;

	sf::RenderTexture mConsoleTexture;
	sf::RenderTexture mScreenTexture;
};