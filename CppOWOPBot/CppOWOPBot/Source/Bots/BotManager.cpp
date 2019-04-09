#include "BotManager.h"
#include <iostream>
#include <fstream>

#include "Util.h"
#include "OWOP/Protocol.h"
#include "PlaceBot.h"

BotManager::BotManager()
{
	try
	{
		// Set logging to be pretty verbose (everything except message payloads)
		//mEndpoint.set_access_channels(websocketpp::log::alevel::all);
		//mEndpoint.clear_access_channels(websocketpp::log::alevel::frame_payload);

		mEndpoint.clear_access_channels(websocketpp::log::alevel::all);
		mEndpoint.clear_error_channels(websocketpp::log::elevel::all);

		// Initialize ASIO
		mEndpoint.init_asio();
		//mEndpoint.set_tls_init_handler(&OnTlsInit);

		mEndpoint.start_perpetual();

		for(int i = 0; i < 4; i ++)
			mRunThreads.emplace_back(std::thread([this]() { mEndpoint.run(); }));
	}
	catch (websocketpp::exception const &)
	{
		//std::cout << "Exception while connecting: " << e.what() << std::endl;
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
		for(auto &x : mRunThreads)
			x.join();

	}
	catch (websocketpp::exception const &)
	{
		//std::cout << "Exception in BotManager Destructor: " << e.what() << std::endl;
	}
}

void BotManager::Connect(const std::string & uri, int numbots, bool useProxy, bool updaterBot)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	for (int i = 0; i < numbots; i++)
	{
		std::unique_ptr<PlaceBot> bot = std::make_unique<PlaceBot>(*this);
		bot->SetRunWorldHandler(updaterBot);

		mBots.emplace_back(std::move(bot));

		std::string proxy = "";
		if(useProxy)
			mProxylist.GetNextProxy(proxy);

		mBots.back()->Connect(uri, proxy);
	}
}



void BotManager::Update(float dt)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);

	static sf::Clock retryPotTimer;

	if (retryPotTimer.getElapsedTime().asSeconds() > 2.5f)
	{
		//possible deadlock
		mRetryPot.Update([this](std::unordered_map<Task::Type, TaskManager::ContainterType> &rMap)
		{
			for (auto &x : rMap)
				mTaskManager.Update(x.first,[&x](TaskManager::ContainterType &cont)
				{
					cont.insert(cont.begin(), x.second.begin(), x.second.end());
					x.second.clear();
				});
		});

		retryPotTimer.restart();
	}

	for (auto &x : mBots)
		x->Update(dt);
}

void BotManager::Draw(sf::RenderTarget & target) const
{
	mWorld.Draw(target);
}

void BotManager::GetAllBots(std::function<void(std::vector<std::unique_ptr<ConnectionBot>> &bots)> func)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	func(mBots);
}

size_t BotManager::GetNumBots() const
{
	std::shared_lock<std::shared_mutex> lock(mMutex);
	return mBots.size();
}

size_t BotManager::GetNumBotsInState(const ConnectionBot::ConnectionState & state) const
{
	std::shared_lock<std::shared_mutex> lock(mMutex);

	size_t count = 0;
	for (auto &x : mBots)
	{
		if (x->GetState() == state)
			count++;
	}

	return count;
}

Ws::ContextPtr BotManager::OnTlsInit(Ws::ConnectionHdl hdl)
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
	catch (std::exception const &)
	{
		//std::cout << "Error in context pointer: " << e.what() << std::endl;
	}
	return ctx;
}
