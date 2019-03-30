#include "ConnectionBot.h"
#include "BotManager.h"
#include "Util.h"
#include "OWOP/Protocol.h"

ConnectionBot::~ConnectionBot()
{
	Disconnect();
}

void ConnectionBot::Connect(const std::string &uri)
{
	Disconnect();
	mUri = uri;

	try
	{
		websocketpp::lib::error_code ec;
		Ws::Client::connection_ptr connection = mManager.GetEndpoint().get_connection(uri, ec);
		if (ec)
		{
			std::cout << "could not create connection because: " << ec.message() << std::endl;
			return;
		}

		connection->set_message_handler(bind(&ConnectionBot::WSMessageHandler, this, ::_1, ::_2));
		connection->set_open_handler(bind(&ConnectionBot::WSOpenHandler, this, ::_1));
		connection->set_close_handler(bind(&ConnectionBot::WSCloseHandler, this, ::_1));
		connection->set_fail_handler(bind(&ConnectionBot::WSFailHandler, this, ::_1));


		//Spoof stuff
		connection->append_header("Origin", "http://ourworldofpixels.com"); //TODO: make dynamic
		connection->replace_header("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/67.0.3396.87 Safari/537.36");

		mManager.GetEndpoint().connect(connection);

		mConnectionHdl = connection;

		mConnectionState = ConnectionState::Connecting;
	}
	catch (websocketpp::exception const & e)
	{
		std::cout << "Exception while connecting: " << e.what() << std::endl;
	}
}

void ConnectionBot::Disconnect()
{
	
	if (mConnectionState == ConnectionState::Disconnected)
		return;

	try
	{
		//mManager.GetEndpoint().pause_reading(mConnectionHdl);
		mManager.GetEndpoint().close(mConnectionHdl, websocketpp::close::status::going_away, "Disconnected");
		mConnectionState = ConnectionState::Disconnecting;
	}
	catch (websocketpp::exception const & e)
	{
		std::cout << "Exception while disconnecting: " << e.what() << std::endl;
	}
}

void ConnectionBot::Update(float dt)
{
	if (mConnectionState == ConnectionState::Connected)
		Disconnect();

	if (mConnectionState == ConnectionState::Disconnected)
		Connect(mUri);
}


ConnectionBot::ConnectionState ConnectionBot::GetState()
{
	return mConnectionState;
}

bool ConnectionBot::IsOWOPConnected()
{
	return mOWOPConnected;
}

void ConnectionBot::WSMessageHandler(Ws::ConnectionHdl hdl, Ws::MessagePtr msg)
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

void ConnectionBot::WSOpenHandler(Ws::ConnectionHdl hdl)
{
	mConnectionState = ConnectionState::Connected;
}

void ConnectionBot::WSCloseHandler(Ws::ConnectionHdl hdl)
{
	mConnectionState = ConnectionState::Disconnected;
}

void ConnectionBot::WSFailHandler(Ws::ConnectionHdl hdl)
{
	try
	{
		Ws::Client::connection_ptr conn = mManager.GetEndpoint().get_con_from_hdl(hdl);
		std::cout << "Connection Fail: " << conn->get_ec().message() << "\n";
	}
	catch (websocketpp::exception const & e)
	{
		std::cout << "Exception in FailHandler: " << e.what() << std::endl;
	}

	mConnectionState = ConnectionState::Disconnected;
}
