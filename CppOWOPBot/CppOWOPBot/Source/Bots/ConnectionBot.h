#pragma once
#include <atomic>
#include <SFML/Graphics.hpp>
#include "WebsocketppDefs.h"
#include "IBot.h"
#include "OWOP/Protocol.h"
#include "OWOP/Bucket.h"
#include "SFExtraMath.h"

class ConnectionBot : public IBot
{
public:
	enum class ConnectionState
	{
		Disconnected,
		Disconnecting,
		Connecting, 
		Connected,
		Joined //when we are able to send cursor commands
	};

	ConnectionBot(BotManager &botManager) : IBot(botManager), mPlaceBucket(0,100){}
	virtual ~ConnectionBot();

	virtual void Connect(const std::string &uri, const std::string &proxyUri = "") override;
	virtual void Disconnect() override;
	virtual void Update(float dt) override;

	bool SendPacket(const Protocol::IC2SMessage &message);

	bool JoinWorld(const std::string &worldName);

	bool SendUpdates(const sf::Vector2i &newPos, OWOP::ToolID toolId = 0,const OWOP::Color &cursorColor = sf::Color(255,255,255));
	bool PlacePixel(const sf::Vector2i &worldPos, const sf::Color &color);
	bool RequestChunk(const sf::Vector2i &chunkPos);

	bool SendChat(const std::string &message);


	//get connection state
	ConnectionState GetState() const;

	//WARNING: MIGHT NOT BE THREADSAFE
	OWOP::CursorData GetCursorData() const;


	//CONFIG
	static void SetRetryDelay(float newDelay);
	static float GetRetryDelay();

protected:
	
	Bucket mPlaceBucket;
	OWOP::CursorData mData;

	std::atomic<ConnectionState> mConnectionState;
	std::string mUri;
	std::string mProxyUri;

	virtual void MessageHandler(const std::shared_ptr<Protocol::IS2CMessage> &message);

	virtual void WSMessageHandler(Ws::ConnectionHdl hdl, Ws::MessagePtr msg);
	virtual void WSOpenHandler(Ws::ConnectionHdl hdl);
	virtual void WSCloseHandler(Ws::ConnectionHdl hdl);
	virtual void WSFailHandler(Ws::ConnectionHdl hdl);

	Ws::ConnectionHdl mConnectionHdl;

private:

	bool mRetrying;
	sf::Clock mRetryClock;

	//GLOBAL CONFIG
	static std::recursive_mutex mConfigMutex;
	static float mRetryDelay;

};