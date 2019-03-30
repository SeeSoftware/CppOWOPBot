#pragma once
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <string>
#include <thread>
#include <vector>
#include <memory>
#include "WebsocketppDefs.h"
#include "ConnectionBot.h"

class BotManager
{
public:

	BotManager();
	~BotManager();

	void Connect(const std::string &uri,int numbots = 1);
	void Update(float dt);

	Ws::Client &GetEndpoint() { return mEndpoint; }
private:

	static Ws::ContextPtr OnTlsInit();

	std::thread mRunThread;

	Ws::Client mEndpoint;
	std::vector<std::unique_ptr<ConnectionBot>> mBots;
};