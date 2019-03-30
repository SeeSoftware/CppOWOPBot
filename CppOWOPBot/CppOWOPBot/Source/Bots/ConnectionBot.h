#pragma once
#include <atomic>
#include "WebsocketppDefs.h"
#include "IBot.h"

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

	ConnectionBot(BotManager &botManager) : IBot(botManager){}
	~ConnectionBot();

	virtual void Connect(const std::string &uri) override;
	virtual void Disconnect() override;
	virtual void Update(float dt) override;

	//get connection state
	ConnectionState GetState();
	//is connected to owop as cursor
	bool IsOWOPConnected();

protected:
	
	

	std::atomic<bool> mOWOPConnected;
	std::atomic<ConnectionState> mConnectionState;
	std::string mUri;

	virtual void WSMessageHandler(Ws::ConnectionHdl hdl, Ws::MessagePtr msg);
	virtual void WSOpenHandler(Ws::ConnectionHdl hdl);
	virtual void WSCloseHandler(Ws::ConnectionHdl hdl);
	virtual void WSFailHandler(Ws::ConnectionHdl hdl);

	Ws::ConnectionHdl mConnectionHdl;
};