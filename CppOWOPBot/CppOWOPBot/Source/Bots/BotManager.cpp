#include "BotManager.h"
#include <iostream>
#include "Util.h"
#include "OWOP/Protocol.h"

BotManager::BotManager()
	:mBot(*this)
{
	try
	{
		// Set logging to be pretty verbose (everything except message payloads)
		mEndpoint.set_access_channels(websocketpp::log::alevel::all);
		mEndpoint.clear_access_channels(websocketpp::log::alevel::frame_payload);

		// Initialize ASIO
		mEndpoint.init_asio();
		mEndpoint.set_tls_init_handler(bind(&OnTlsInit));

		// Register our message handler
		mEndpoint.set_message_handler(bind(&BotManager::OnMessage, this, ::_1, ::_2));
	}
	catch (websocketpp::exception const & e)
	{
		std::cout << "Exception while connecting: " << e.what() << std::endl;
	}
}

void BotManager::Connect(const std::string & uri)
{
	mUri = uri;
		

		
		
	
}

void BotManager::Update(float dt)
{
}

void BotManager::Poll()
{
	mWsClient.poll();
}

void BotManager::OnMessage(websocketpp::connection_hdl hdl, Ws::MessagePtr msg)
{
	std::cout << Util::HexDump((uint8_t*)msg->get_payload().data(), msg->get_payload().size()).str();

	std::shared_ptr<Protocol::IS2CMessage> message = Protocol::ParsePacket(msg);
	
	switch (message->opcode)
	{
		case Protocol::PacketOpCode::Captcha:
		{
			std::cout << "Received Captcha Request \n";
			std::shared_ptr<Protocol::CaptchaRequest> castMsg = Protocol::DowncastMessage<Protocol::CaptchaRequest>(message);

			std::cout << "State: " << (int)castMsg->state << "\n";

			break;
		}
		default:
			std::cout << "Unknown message: " << (int)message->opcode << "\n";  
			break;
	}

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
