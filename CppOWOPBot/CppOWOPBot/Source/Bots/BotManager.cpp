#include "BotManager.h"
#include <iostream>
#include "Util.h"
#include "OWOP/Protocol.h"

BotManager::BotManager()
{
	try
	{
		// Set logging to be pretty verbose (everything except message payloads)
		//mEndpoint.set_access_channels(websocketpp::log::alevel::all);
		//mEndpoint.clear_access_channels(websocketpp::log::alevel::frame_payload);

		mEndpoint.clear_access_channels(websocketpp::log::alevel::all);

		// Initialize ASIO
		mEndpoint.init_asio();
		mEndpoint.set_tls_init_handler(bind(&OnTlsInit));

		mEndpoint.start_perpetual();
		mRunThread = std::thread([this]() { mEndpoint.run(); });
	}
	catch (websocketpp::exception const & e)
	{
		std::cout << "Exception while connecting: " << e.what() << std::endl;
	}
}

BotManager::~BotManager()
{
	try
	{
		//disconnect all bots
		for (auto &x : mBots)
			x->Disconnect();

		mEndpoint.stop_perpetual();
		mRunThread.join();

	}
	catch (websocketpp::exception const & e)
	{
		std::cout << "Exception in BotManager Destructor: " << e.what() << std::endl;
	}
}

void BotManager::Connect(const std::string & uri, int numbots)
{
	for (int i = 0; i < numbots; i++)
	{
		mBots.emplace_back(std::make_unique<ConnectionBot>(*this));
		mBots.back()->Connect(uri);
	}
}



void BotManager::Update(float dt)
{
	for (auto &x : mBots)
		x->Update(dt);
}


Ws::ContextPtr BotManager::OnTlsInit()
{
	// establishes a SSL connection
	Ws::ContextPtr ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);

	try
	{
		ctx->set_options(boost::asio::ssl::context::default_workarounds |
						 boost::asio::ssl::context::no_sslv2 |
						 boost::asio::ssl::context::no_sslv3 |
						 boost::asio::ssl::context::single_dh_use);
	}
	catch (std::exception &e)
	{
		std::cout << "Error in context pointer: " << e.what() << std::endl;
	}
	return ctx;
}
