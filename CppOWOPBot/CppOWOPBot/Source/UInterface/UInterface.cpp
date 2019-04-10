#include "UInterface.h"
#include <fstream>

#include "Imgui/IncImgui.h"
#include "Imgui/TextEditor.h"

void UInterface::DrawGui()
{
	mLuaEnv.RunHook("RENDER");

	StartFullscreen();
	mIsFocused = !ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) && !ImGui::IsWindowHovered(ImGuiFocusedFlags_AnyWindow);
	EndFullscreen();

	///BOT MANAGER
	if (ImGui::Begin("Bot Manager"))
	{
		ImGui::Text("Bots: %d   Connected Bots: %d", mManager.GetNumBots(), mManager.GetNumBotsInState(ConnectionBot::ConnectionState::Joined));
		ImGui::Text("Num Proxies: %d", mManager.GetProxyList().GetNumProxies());

		static int botAmmount = 1;
		static int cursorsPerProxy = 1;
		static bool useProxy = true;
		static bool updaterBot = true;

		ImGui::InputInt("Num Bots", &botAmmount);
		if (ImGui::InputInt("Cursors Per Proxy", &cursorsPerProxy))
			mManager.GetProxyList().SetSettings(cursorsPerProxy);

		ImGui::Checkbox("Use Proxy", &useProxy);

		if (ImGui::Button("Add"))
		{
			mManager.Connect("wss://ourworldofpixels.com", botAmmount, useProxy); //ws://104.237.150.24:1337 //wss://ourworldofpixels.com
		}
		ImGui::SameLine();
		if (ImGui::Button("Remove"))
		{
			mManager.GetAllBots([](std::vector<std::unique_ptr<ConnectionBot>> &bots)
			{
				for (int i = 0; i < botAmmount && bots.size() > 0; i++)
				{
					bots.pop_back();
				}
			});
		}

		ImGui::SameLine();
	}
	ImGui::End();
	/////////////////////////////

	///////PROXY MANAGER
	if (ImGui::Begin("Proxy Manager"))
	{
		static std::string filename = "proxylist.txt";
		ImGui::InputText("Filename", &filename);
		
		if (ImGui::Button("Add"))
		{
			mManager.GetProxyList().LoadProxyfile(filename);
		}
		ImGui::SameLine();

		if (ImGui::Button("Clear"))
		{
			mManager.GetProxyList().ClearProxyList();
		}

		if (ImGui::BeginChild((ImGuiID)1))
		{
			mManager.GetProxyList().Update([](std::deque<std::string> &proxies)
			{
				for (const auto &x : proxies)
				{
					ImGui::Text(x.c_str());
				}
			});
		}
		ImGui::EndChild();
	}
	ImGui::End();
	////////////////

	///////MISC
	if (ImGui::Begin("Misc"))
	{
		ImGui::Text("Number Tasks: %d", mManager.GetTaskManager().GetTotalNumTasks());
		if (ImGui::Button("Clear World"))
			mManager.GetWorld().ClearWorld();

		if (ImGui::Button("Clear Tasks"))
			mManager.GetTaskManager().ClearTasks();

	}
	ImGui::End();
	//////////////

	/////TOOLS
	if (ImGui::Begin("Tools"))
	{
		for (const auto &x : mTools)
		{
			ImGui::BeginGroup();
			ImGui::Text(x->ToolName().c_str());
			ImGui::Image(x->ToolImage(), sf::Color::White, sf::Color::Black);
			ImGui::EndGroup();
			ImGui::SameLine();
		}
	}
	ImGui::End();
	////////////////

	////TEXT EDITOR
	{
		static TextEditor editor;
		static bool editorSetup = true;
		static const std::string SavePath = "Scripts/";
		static std::string scriptFilename = "Default.lua";

		static auto loadFile = [](const std::string &name)
		{
			std::ifstream file(name);

			if (file.bad())
				return;

			std::string line;
			std::string text = "";

			while (std::getline(file, line))
				text += line + "\n";

			editor.SetText(text);
		};

		static auto saveFile = [](const std::string &name)
		{
			std::ofstream file(name);

			if (file.bad())
				return;
			
			file << editor.GetText();
			file.close();
		};


		if (editorSetup)
		{
			editor.SetLanguageDefinition(TextEditor::LanguageDefinition::OWOPLua());
			editorSetup = false;

			loadFile(SavePath + scriptFilename);
			mLuaEnv.RunScript(editor.GetText());
		}


		if (ImGui::Begin("Lua Editor", (bool*)0, ImGuiWindowFlags_NoScrollbar))
		{
			editor.Render("Lua", ImVec2(0, ImGui::GetWindowContentRegionMax().y - 70));

			ImGui::InputText("Filename", &scriptFilename);

			
			if (ImGui::Button("Run"))
				mLuaEnv.RunScript(editor.GetText());

			ImGui::SameLine(0.0f,20.0f);

			if (ImGui::Button("Load"))
				loadFile(SavePath + scriptFilename);

			ImGui::SameLine();

			if (ImGui::Button("Save"))
				saveFile(SavePath + scriptFilename);

			

			if (ImGui::IsKeyPressed(sf::Keyboard::S, false) && ImGui::GetIO().KeyCtrl) //save pressed
				saveFile(SavePath + scriptFilename);

		}
		ImGui::End();

	}
	/////

}

void UInterface::ProcessEvent(const sf::Event &e)
{
	if (!mIsFocused)
		return; 

	switch (e.type)
	{
		case sf::Event::KeyPressed:
		{
			break;
		}

		case sf::Event::MouseWheelScrolled:
		{
			static float totalZoom = 1.0f;
			if (e.mouseWheelScroll.delta > 0.0f)
			{
				totalZoom *= 0.5f;
				mView.zoom(0.5f);
			}
			else if(totalZoom <= 1.0f)
			{
				totalZoom *= 2.0f;
				mView.zoom(2.0f);
			}

			mTarget.setView(mView);
			break;
		}

		case sf::Event::MouseButtonPressed:
		{
			if (e.mouseButton.button == sf::Mouse::Button::Left && sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
			{
				mDragging = true;
				mDragStart = mTarget.mapPixelToCoords(sf::Vector2i(e.mouseButton.x, e.mouseButton.y));
				mViewCenter = mView.getCenter();
				mDragView = mView;
			}
			
			break;
		}

		case sf::Event::MouseButtonReleased:
		{
			if (e.mouseButton.button == sf::Mouse::Button::Left)
				mDragging = false;

			break;
		}

		default:
			break;
	}
}

void UInterface::Update(float dt)
{

	mLuaEnv.RunHook("THINK",(double)dt);

	////Everything after this only runs if we are focused
	if (!mTarget.hasFocus() || !mIsFocused)
		return;

	////////////////Dragging
	if(mDragging)
		mTarget.setView(mDragView);

	sf::Vector2f cursorPos = mTarget.mapPixelToCoords(sf::Mouse::getPosition(mTarget));
	
	if (mDragging)
	{
		mView.setCenter(mViewCenter + (mDragStart - cursorPos));
		mTarget.setView(mView);
	}
	//////////////////

	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
	{
		mManager.GetTaskManager().Update(Task::PlacePixel, [this,&cursorPos](TaskManager::ContainterType &cont)
		{
			if (cont.size() > 0)
				return;

			for (int y = 0; y < 64; y++)
				for (int x = 0; x < 64; x++)
					cont.push_back(Task::Task(Task::PlacePixel, (sf::Vector2i)cursorPos + sf::Vector2i(x, y), sf::Color(255,255,255)));
		});
	}


}

void UInterface::StartFullscreen()
{
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::Begin("BackBuffer", 0, ImVec2(0, 0), 0.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs);// | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing);

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->DrawList->PushClipRectFullScreen();

	ImGui::SetWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);
	ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y), ImGuiSetCond_Always);
}

void UInterface::EndFullscreen()
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->DrawList->PushClipRectFullScreen();

	ImGui::End();
	ImGui::PopStyleColor(2);
}
