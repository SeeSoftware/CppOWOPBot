#include "ConsoleTool.h"
#include "Imgui/IncImgui.h"
#include "ConsoleHelper.h"

ConsoleTool::ConsoleTool(UInterface & ui, BotManager & manager, sf::RenderWindow & target) : ITool(ui, manager, target) 
{ 
	mFont.loadFromFile("Fonts/4x5-pixel-font.ttf");
	mScreenTexture.create(1024, 1024, sf::ContextSettings());

	sf::Glyph glyph = mFont.getGlyph('W', FONT_SIZE, false);
	mFontSpacing.x = (int)(glyph.bounds.width - glyph.bounds.left) + 1;
	mFontSpacing.y = (int)(glyph.bounds.height - glyph.bounds.top) - 2;

	UpdateResolution();
}


void ConsoleTool::ConsoleSetPos(const sf::Vector2i & pos)
{
	mCurrentTextPos = pos;
}

void ConsoleTool::ConsolePutc(char c)
{
	c = toupper(c);

	if (c == '\0')
		return;
	if (c == '\n')
	{
		mCurrentTextPos.x = 0;
		mCurrentTextPos.y++;
		return;
	}

	if (mCurrentTextPos.x >= mResolution.x)
	{
		mCurrentTextPos.x = 0;
		mCurrentTextPos.y++;
	}
	if (mCurrentTextPos.y >= mResolution.y)
		ConsoleShiftUp((mCurrentTextPos.y - mResolution.y) + 1);

	mTextBuffer[mCurrentTextPos.x + mCurrentTextPos.y * mResolution.x] = c;
	mCurrentTextPos.x++;
}

void ConsoleTool::ConsolePuts(const std::string & text)
{
	for (const char &x : text)
	{
		ConsolePutc(x);
	}
}

void ConsoleTool::ConsoleShiftUp(int ammount)
{
	for(int y = 0; y < mResolution.y; y++)
		for (int x = 0; x < mResolution.x; x++)
		{
			int newy = y + ammount;
			char newc = '\0';
			if (newy >= 0 && newy < mResolution.y)
				newc = mTextBuffer[x + newy * mResolution.x];

			mTextBuffer[x + y * mResolution.x] = newc;
		}

	mCurrentTextPos.y -= ammount;
}


void ConsoleTool::ClearConsole()
{
	mTextBuffer.clear();
	mTextBuffer.resize(mResolution.x * mResolution.y);
	mCurrentTextPos = sf::Vector2i(0,0);
}

void ConsoleTool::DrawGui()
{
	if (ImGui::Begin("Console Tool"))
	{
		ImGui::Checkbox("Enabled", &mIsEnabled);

		ImGui::InputInt("x:", &mPosition.x);
		ImGui::SameLine();
		ImGui::InputInt("y:", &mPosition.y);

		if (ImGui::InputInt("w:", &mResolution.x))
			UpdateResolution();

		ImGui::SameLine();
		if (ImGui::InputInt("h:", &mResolution.y))
			UpdateResolution();


		static std::string cmd;
		if (ImGui::InputText("Command", &cmd,ImGuiInputTextFlags_EnterReturnsTrue))
		{
			ConsolePuts(cmd);
		}

	}
	ImGui::End();


	/*sf::RectangleShape shape((sf::Vector2f)mConsoleTexture.getSize());
	shape.setTexture(&mConsoleTexture.getTexture());
	shape.setPosition(sf::Vector2f(0,0));
	shape.setFillColor(sf::Color(255, 255, 255, 255));

	mTarget.draw(shape);*/
}

void ConsoleTool::ProcessEvent(const sf::Event & e)
{
}

void ConsoleTool::Update(float dt)
{
	if (!mIsEnabled)
		return;

	static int left = 0, top = 0, width = 0, height = 0;
	ConsoleHelper::GetConsoleRect(left, top, width, height);
	if (sf::Vector2i(width, height) != mResolution)
	{
		mResolution = sf::Vector2i(width, height);
		UpdateResolution();
	}


	UpdateAndDrawConsole(dt);

	if (mManager.GetTaskManager().GetNumTasks(Task::PlacePixel) > 0)
		return;

	sf::Image mImage = mConsoleTexture.getTexture().copyToImage();

	//Draw image in owop;
	//FIX: I DONT FUCKING UNDERSTAND WHY THE IMAGE IS Y MIRRORED SO HERE IS A HAXKY FIX I DONT CARE ANYMORE
	mManager.GetWorld().Update([&](UnsafeWorld &world)
	{
		mManager.GetTaskManager().Update([&](UnsafeTaskManager &manager)
		{
			for (uint32_t y = 0; y < mImage.getSize().y; y++)
				for (uint32_t x = 0; x < mImage.getSize().x; x++)
				{
					sf::Color imagePixel = mImage.getPixel(x, (mImage.getSize().y - 1) - y);
					if (world.GetPixel(mPosition + sf::Vector2i(x, y)) == imagePixel)
						continue;

					manager.PushTask(Task(Task::PlacePixel, mPosition + sf::Vector2i(x, y), imagePixel));
				}
		});
	});

}

std::string ConsoleTool::ToolName() const
{
	return "Console Tool";
}

sf::Texture & ConsoleTool::ToolImage() const
{
	static sf::Texture texture;
	static bool loaded = false;
	if (!loaded)
	{
		texture.loadFromFile("Images/ImageTool.png");
		loaded = true;
	}

	return texture;
}

void ConsoleTool::UpdateResolution()
{
	mConsoleTexture.create(mResolution.x * mFontSpacing.x, mResolution.y * mFontSpacing.y, sf::ContextSettings());
	mConsoleTexture.clear();

	ClearConsole();
}

void ConsoleTool::UpdateAndDrawConsole(float dt)
{
	//kinda defeates purpose of everything else but eh
	AttachCMD();
	UpdateTextBufferFromCMD();


	mConsoleTexture.clear(sf::Color(0, 0, 0, 255));


	//TEXT DRAWING
	/*for (int y = 0; y < mResolution.y; y++)
	{
		std::string line(&mTextBuffer[y * mResolution.x], mResolution.x);
		line = std::string(line.c_str());


		sf::Text text;
		text.setString(line);
		text.setFillColor(sf::Color(255, 255, 255, 255));
		text.setFont(mFont);
		text.setCharacterSize(FONT_SIZE);
		text.setScale(sf::Vector2f(1.0f,1.0f)); //to unsmooth pixel edges
		text.setPosition(sf::Vector2f(0, (float)(y*mFontSpacing.y)));

		mConsoleTexture.draw(text);
	}*/

	//TEXT DRAWING PER CHARACTER (fixes spacing problem)
	for (int y = 0; y < mResolution.y; y++)
		for (int x = 0; x < mResolution.x; x++)
		{
			char character = mTextBuffer[x + y * mResolution.x];
			if (character == '\0' || character == ' ')
				continue;

			sf::Text text;
			text.setString(character);
			text.setFillColor(sf::Color(255, 255, 255, 255));
			text.setFont(mFont);
			text.setCharacterSize(FONT_SIZE);
			text.setScale(sf::Vector2f(1.0f, 1.0f)); //to unsmooth pixel edges
			text.setPosition(sf::Vector2f((float)(x*mFontSpacing.x), (float)(y*mFontSpacing.y)));

			mConsoleTexture.draw(text);
		}

	/*//PIXEL DRAWING
	for(int y = 0; y < mResolution.y; y++)
		for (int x = 0; x < mResolution.x; x++)
		{
			char c = mTextBuffer[x + y * mResolution.x];
			sf::Color col = (c == ' ' || c == '\n' || c == '\0') ? sf::Color(0, 0, 0, 255) : sf::Color(255, 255, 255, 255);

			sf::RectangleShape rect((sf::Vector2f)mFontSpacing);
			rect.setFillColor(col);
			rect.setPosition(sf::Vector2f((float)x*mFontSpacing.x, (float)y*mFontSpacing.y));
			mConsoleTexture.draw(rect);
		}*/
}



void ConsoleTool::AttachCMD()
{	
	static bool mIsAttached = false;

	if (mIsAttached)
		return;
	mIsAttached = true;

	// additional information
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	// set the size of the structures
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));

	si.cb = sizeof(si);

	CreateProcessA("C:\\Windows\\System32\\cmd.exe",   // the path
		NULL,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
	);

	//FreeConsole();
	AttachConsole(pi.dwProcessId);

	/*freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);*/
}

void ConsoleTool::UpdateTextBufferFromCMD()
{
	int left, top, width, height;

	ConsoleHelper::GetConsoleRect(left, top, width, height);

	std::vector<std::string> lines;

	DWORD size = width * height;
	char *buff = new char[size];
	ConsoleHelper::ReadConsoleBuffer(buff, size, { 0,(short)top });


	for (int count = 0; count < (int)size; count += width)
	{
		lines.emplace_back(buff + count, std::min(width, (int)size - (int)count) - 1);
	}

	ClearConsole();
	//cut away unseen things
	for (int y = 0; y < mResolution.y; y++)
	{
		std::string line = "";
		if (y >= 0 && y < lines.size())
			line = lines[y];

		memcpy(&mTextBuffer[y* mResolution.x], line.data(), std::min(line.size(), (size_t)mResolution.x));
	}

	delete[] buff;

}
