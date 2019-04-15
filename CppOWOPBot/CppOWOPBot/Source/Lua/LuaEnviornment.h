#pragma once
//saftety things (should probably only be in debug but its better to be safe than sorry :)
#define SOL_CHECK_ARGUMENTS 1
#define SOL_SAFE_NUMERICS 0
/*#define SOL_SAFE_USERTYPE 1
#define SOL_SAFE_REFERENCES 1
#define SOL_SAFE_FUNCTION_CALLS 1
*/

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
	~LuaEnviornment()
	{
		mHooks.clear();
	}
	
	bool RunScript(const std::string &script, std::string *errorString = nullptr);

	template<typename ...Args>
	void RunHook(const std::string &hookName, Args &&...args)
	{
		for (auto &x : mHooks[hookName])
		{
			if (x.second)
				x.second(std::forward<Args>(args)...);
		}

	}

	void RunTimers();


	sol::state &GetState() { return mState; }

private:

	struct Timer
	{
		sf::Clock clock;
		float delay;
		int repetitions;
		sol::protected_function callback;
	};

	void InitEnviornment();

	std::unordered_map<std::string,std::unordered_map<std::string, sol::protected_function>> mHooks;
	std::unordered_map<std::string, Timer> mTimer;

	sf::RenderWindow &mTarget;
	BotManager &mManager;
	sol::state mState;
};