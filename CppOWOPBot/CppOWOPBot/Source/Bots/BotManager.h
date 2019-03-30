#pragma once
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <string>
#include "WebsocketppDefs.h"
#include "ConnectionBot.h"

class BotManager
{
public:

	BotManager();
	~BotManager() = default;

	void Connect(const std::string &uri);
	void Update(float dt);

	Ws::Client &GetEndpoint() { return mEndpoint; }
	const std::string &GetCurrentUri() { return mUri; }

private:

	void OnMessage(websocketpp::connection_hdl hdl, Ws::MessagePtr msg);
	static Ws::ContextPtr OnTlsInit();

	std::string mUri;

	Ws::Client mEndpoint;
	ConnectionBot mBot;
};