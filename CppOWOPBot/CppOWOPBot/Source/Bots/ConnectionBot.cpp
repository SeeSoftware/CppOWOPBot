#include "ConnectionBot.h"
#include "BotManager.h"
#include "Util.h"



ConnectionBot::~ConnectionBot()
{
	Disconnect();
	try
	{
		Ws::Client::connection_ptr ptr = mManager.GetEndpoint().get_con_from_hdl(mConnectionHdl);

		//TODO: Better method to fix crash when websockerpp tries to use something from destructed Bot
		ptr->set_message_handler(bind([]() {}));
		ptr->set_open_handler(bind([]() {}));
		ptr->set_close_handler(bind([]() {}));
		ptr->set_fail_handler(bind([]() {}));
	}
	catch (websocketpp::exception const &)
	{

	}
}

void ConnectionBot::Connect(const std::string &uri, const std::string &proxyUri) //TODO: make dynamic
{
	Disconnect();
	mUri = uri;
	mProxyUri = proxyUri;

	try
	{
		websocketpp::lib::error_code ec;
		Ws::Client::connection_ptr connection = mManager.GetEndpoint().get_connection(uri, ec);
		if (ec)
		{
			//std::cout << "could not create connection because: " << ec.message() << std::endl;
			return;
		}

		if(proxyUri != "")
			connection->set_proxy("https://"+proxyUri);

		connection->set_message_handler(bind(&ConnectionBot::WSMessageHandler, this, ::_1, ::_2));
		connection->set_open_handler(bind(&ConnectionBot::WSOpenHandler, this, ::_1));
		connection->set_close_handler(bind(&ConnectionBot::WSCloseHandler, this, ::_1));
		connection->set_fail_handler(bind(&ConnectionBot::WSFailHandler, this, ::_1));


		//Spoof stuff
		connection->append_header("Origin", "http://ourworldofpixels.com"); 
		connection->replace_header("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/67.0.3396.87 Safari/537.36");

		mManager.GetEndpoint().connect(connection);

		mConnectionHdl = connection;

		mConnectionState = ConnectionState::Connecting;
	}
	catch (websocketpp::exception const &)
	{
		//std::cout << "Exception while connecting: " << e.what() << std::endl;
	}
}

void ConnectionBot::Disconnect()
{
	
	if (mConnectionState < ConnectionState::Connected)
		return;

	try
	{
		//mManager.GetEndpoint().pause_reading(mConnectionHdl);
		mManager.GetEndpoint().close(mConnectionHdl, websocketpp::close::status::going_away, "Disconnected");
		mConnectionState = ConnectionState::Disconnecting;
	}
	catch (websocketpp::exception const &)
	{
		//std::cout << "Exception while disconnecting: " << e.what() << std::endl;
	}
}

static sf::Clock timer;

void ConnectionBot::Update(float dt)
{
	mPlaceBucket.Update();

	if (mConnectionState == ConnectionState::Disconnected)
		Connect(mUri, mProxyUri);
}

bool ConnectionBot::SendPacket(const Protocol::IC2SMessage& message)
{
	std::error_code ec;
	std::vector<uint8_t> data = message.Serialize();

	//std::cout << Util::HexDump(data.data(), data.size()).str() << "\n";

	mManager.GetEndpoint().send(mConnectionHdl, data.data(), data.size(), message.WSOpcode(),ec);
	
	if (ec)
	{
		//std::cout << "Error Sending Packet: " << ec.message() << "\n"; 
		return false;
	}
	return true;

}



bool ConnectionBot::JoinWorld(const std::string & worldName)
{
	if (mConnectionState < ConnectionState::Connected)
		return false;

	//std::cout << "Joining world " << worldName << "\n";

	Protocol::JoinWorld msg(worldName);
	return SendPacket(msg);
}

bool ConnectionBot::SendUpdates(const sf::Vector2i & newWorldPos, OWOP::ToolID toolId, const sf::Color &cursorColor)
{
	if (mConnectionState < ConnectionState::Joined)
		return false;

	Protocol::UpdateCursor msg(newWorldPos*16,toolId, cursorColor);
	return SendPacket(msg);
}

bool ConnectionBot::PlacePixel(const sf::Vector2i &worldPos, const sf::Color & color)
{
	mPlaceBucket.Update();
	
	if (color.a == 0)
		return true;

	if (mConnectionState >= ConnectionState::Joined && mPlaceBucket.Spend(1))
	{
		int distx = worldPos.x / OWOP::CHUNK_SIZE - mData.GetWorldPos().x / OWOP::CHUNK_SIZE;
		int disty = worldPos.y / OWOP::CHUNK_SIZE - mData.GetWorldPos().y / OWOP::CHUNK_SIZE;
		float dist = sqrtf((float)(distx*distx) + (float)(disty*disty));

		/*if (dist >= 3)
		{
			sf::Vector2i rpos = sf::Vector2i(Util::div_floor(worldPos.x, OWOP::CHUNK_SIZE), Util::div_floor(worldPos.y, OWOP::CHUNK_SIZE)) / sf::Vector2i(3, 3);
			SendUpdates(rpos * sf::Vector2i(3* OWOP::CHUNK_SIZE, 3* OWOP::CHUNK_SIZE));
		}*/

		SendUpdates(worldPos);
		Protocol::UpdatePixel msg(worldPos, color);
		return SendPacket(msg);
	}
	return false;
}

bool ConnectionBot::RequestChunk(const sf::Vector2i & chunkPos)
{
	//TODO: more bucket checks
	if (mConnectionState < ConnectionState::Joined)
		return false;

	Protocol::RequestChunk msg(chunkPos);
	return SendPacket(msg);
	
}

bool ConnectionBot::SendChat(const std::string & message)
{
	//TODO: more bucket checks
	if (mConnectionState < ConnectionState::Joined)
		return false;
	
	Protocol::SendChat msg(message);
	return SendPacket(msg);

}

ConnectionBot::ConnectionState ConnectionBot::GetState() const
{
	return mConnectionState;
}

OWOP::CursorData ConnectionBot::GetCursorData() const
{
	//TODO:
	//WARNING: MIGHT NOT BE THREADSAFE
	return mData;
}

void ConnectionBot::MessageHandler(const std::shared_ptr<Protocol::IS2CMessage>& message)
{
	switch (message->opcode)
	{
		case Protocol::PacketOpCode::SetID:
		{
			//std::cout << "Received SetID\n";
			std::shared_ptr<Protocol::SetID> castMsg = Protocol::DowncastMessage<Protocol::SetID>(message);

			//std::cout << "id: " << castMsg->id << "\n";
			break;
		}

		case Protocol::PacketOpCode::SetPQuota:
		{
			//std::cout << "Received SetPQuota\n";
			std::shared_ptr<Protocol::SetPQuota> castMsg = Protocol::DowncastMessage<Protocol::SetPQuota>(message);
			
			/*std::cout << "per: " << castMsg->per << "\n";
			std::cout << "rate: " << castMsg->rate << "\n";*/

			mPlaceBucket.Per = castMsg->per;
			mPlaceBucket.Rate = castMsg->rate;
			mPlaceBucket.Reset();

			mConnectionState = ConnectionState::Joined;
			break;
		}

		case Protocol::PacketOpCode::Captcha:
		{
			//std::cout << "Received Captcha Request \n";
			std::shared_ptr<Protocol::CaptchaRequest> castMsg = Protocol::DowncastMessage<Protocol::CaptchaRequest>(message);

			//std::cout << "State: " << (int)castMsg->state << "\n";

			switch (castMsg->state)
			{
				case OWOP::CaptchaState::CA_OK:
					JoinWorld("bot");
					break;
			}

			break;
		}
		default:
			//std::cout << "Unknown message: " << (int)message->opcode << "\n";
			break;
	}

	//mManager.GetWorld().HandlePacket(message);
}

void ConnectionBot::WSMessageHandler(Ws::ConnectionHdl hdl, Ws::MessagePtr msg)
{
	//std::cout << Util::HexDump((uint8_t*)msg->get_payload().data(), msg->get_payload().size()).str();
	std::shared_ptr<Protocol::IS2CMessage> message = Protocol::ParsePacket(msg);
	MessageHandler(message);
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
		//std::cout << "Connection Fail: " << conn->get_ec().message() << "\n";
	}
	catch (websocketpp::exception const &)
	{
		//std::cout << "Exception in FailHandler: " << e.what() << std::endl;
	}

	mConnectionState = ConnectionState::Disconnected;
}
