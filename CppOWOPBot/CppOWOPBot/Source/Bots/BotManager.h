#pragma once
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <string>
#include "WebsocketppDefs.h"

class BotManager
{
public:

	BotManager() = default;
	~BotManager() = default;


	void Connect(const std::string &uri);
	void Poll();

private:

	void OnMessage(websocketpp::connection_hdl hdl, Ws::MessagePtr msg);
	static Ws::ContextPtr OnTlsInit();

	Ws::Client mWsClient;
	Ws::Client::connection_ptr mConnection;

};