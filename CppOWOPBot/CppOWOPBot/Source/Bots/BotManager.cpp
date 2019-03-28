#include "BotManager.h"
#include <iostream>
#include "Util.h"
#include "OWOP/Protocol.h"

void BotManager::Connect(const std::string & uri)
{
	try
	{
		// Set logging to be pretty verbose (everything except message payloads)
		mWsClient.set_access_channels(websocketpp::log::alevel::all);
		mWsClient.clear_access_channels(websocketpp::log::alevel::frame_payload);
		
		// Initialize ASIO
		mWsClient.init_asio();
		mWsClient.set_tls_init_handler(bind(&OnTlsInit));

		// Register our message handler
		mWsClient.set_message_handler(bind(&BotManager::OnMessage, this, ::_1, ::_2));

		websocketpp::lib::error_code ec;
		mConnection = mWsClient.get_connection(uri, ec);
		if (ec)
		{
			std::cout << "could not create connection because: " << ec.message() << std::endl;
			return;
		}


		//Spoof stuff
		mConnection->append_header("Origin", "http://ourworldofpixels.com");
		mConnection->replace_header("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/67.0.3396.87 Safari/537.36");

		// Note that connect here only requests a connection. No network messages are
		// exchanged until the event loop starts running in the next line.
		mWsClient.connect(mConnection);
	}
	catch (websocketpp::exception const & e)
	{
		std::cout << "Exception while connecting: " << e.what() << std::endl;
	}
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
