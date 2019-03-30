#include "ConnectionBot.h"
#include "BotManager.h"

ConnectionBot::~ConnectionBot()
{
	Disconnect();
}

void ConnectionBot::Connect()
{
	try
	{
		websocketpp::lib::error_code ec;
		Ws::Client::connection_ptr connection = mManager.GetEndpoint().get_connection(mManager.GetCurrentUri(), ec);
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

	}
	catch (websocketpp::exception const & e)
	{
		std::cout << "Exception while connecting: " << e.what() << std::endl;
	}
}

void ConnectionBot::Disconnect()
{
	try
	{
		mManager.GetEndpoint().pause_reading(mConnectionHdl);
		mManager.GetEndpoint().close(mConnectionHdl, websocketpp::close::status::normal, "Disconnected");
	}
	catch (websocketpp::exception const & e)
	{
		std::cout << "Exception while disconnecting: " << e.what() << std::endl;
	}
}

void ConnectionBot::Update(float dt)
{
}

bool ConnectionBot::IsWsConnected()
{
	return mWsConnected;
}

bool ConnectionBot::IsOWOPConnected()
{
	return mOWOPConnected;
}

void ConnectionBot::WSMessageHandler(Ws::ConnectionHdl hdl, Ws::MessagePtr msg)
{
}

void ConnectionBot::WSOpenHandler(Ws::ConnectionHdl hdl)
{
	mWsConnected = true;
}

void ConnectionBot::WSCloseHandler(Ws::ConnectionHdl hdl)
{
	mWsConnected = false;
}

void ConnectionBot::WSFailHandler(Ws::ConnectionHdl hdl)
{
	mWsConnected = false;
}
