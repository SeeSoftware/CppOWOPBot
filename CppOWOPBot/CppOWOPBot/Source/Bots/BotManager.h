#pragma once
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

#include <string>
#include <vector>

#include <atomic>
#include <thread>
#include <memory>
#include <mutex>
#include <functional>
#include <shared_mutex>

#include "WebsocketppDefs.h"
#include "ConnectionBot.h"
#include "World/World.h"
#include "TaskManager/TaskManager.h"
#include "TaskManager/PixelCheck.h"
#include "ProxyManager/Proxylist.h"

class BotManager
{
public:
	BotManager();
	~BotManager();

	void Connect(const std::string &uri,int numbots = 1, bool useProxy = false, bool updaterBot = true);
	void Update(float dt);
	void Draw(sf::RenderTarget &target) const;

	//this is dangerous and its easy to cause a deadlock
	//you MUST NOT call any other function from this instance when inside the callback
	void UpdateBots(std::function<void(std::vector<std::unique_ptr<ConnectionBot>> &bots)> func);

	size_t GetNumBots() const;
	size_t GetNumBotsInState(const ConnectionBot::ConnectionState &state) const;

	Ws::Client &GetEndpoint() { return mEndpoint; }
	World &GetWorld() { return mWorld; }
	TaskManager &GetTaskManager() { return mTaskManager; }
	//tasks in retry pot will get reput into the taskManager but with a delay (so you can check if a task was successfully furfilled
	PixelCheck &GetPixelCheck() { return mPixelCheck; }
	Proxylist &GetProxyList() { return mProxylist; }

private:

	static Ws::ContextPtr OnTlsInit(Ws::ConnectionHdl hdl);

	
	mutable std::shared_mutex mMutex;

	std::vector<std::thread> mRunThreads;
	std::atomic<bool> mExitThread = false;

	World mWorld;
	TaskManager mTaskManager;
	PixelCheck mPixelCheck;
	Proxylist mProxylist;

	Ws::Client mEndpoint;
	std::vector<std::unique_ptr<ConnectionBot>> mBots;
};