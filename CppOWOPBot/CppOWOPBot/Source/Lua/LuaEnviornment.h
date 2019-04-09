#pragma once
#include <sol.hpp>
#include <SFML/Graphics.hpp>
#include <functional>
#include <unordered_map>
#include <string>
#include <memory>
#include "Imgui/IncImgui.h"

#include "Bots/BotManager.h"


class LuaEnviornment
{
public:
	LuaEnviornment(BotManager &manager, sf::RenderWindow &target) :mManager(manager), mTarget(target)
	{
		InitEnviornment();
	};
	~LuaEnviornment() = default;
	
	bool RunScript(const std::string &script, std::string *errorString = nullptr);

	template<typename ...Args>
	void RunHook(const std::string &hookName, Args &&...args)
	{
		try
		{
			for (auto &x : mHooks[hookName])
			{
				if (x.second)
					x.second(std::forward<Args>(args)...);
			}
		}
		catch (sol::error &err)
		{
			std::cout << err.what() << "\n";
		}
	}

	sol::state &GetState() { return mState; }

private:

	void InitEnviornment();

	std::unordered_map<std::string,std::unordered_map<std::string, sol::function>> mHooks;


	sf::RenderWindow &mTarget;
	BotManager &mManager;
	sol::state mState;
};