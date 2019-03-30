#pragma once
#include <atomic>
#include "WebsocketppDefs.h"
#include "IBot.h"

class ConnectionBot : public IBot
{
public:

	ConnectionBot(BotManager &botManager) : IBot(botManager){}
	~ConnectionBot();

	virtual void Connect() override;
	virtual void Disconnect() override;
	virtual void Update(float dt) override;

	//is connected to websocket
	bool IsWsConnected();
	//is connected to owop as cursor
	bool IsOWOPConnected();

protected:
	
	std::atomic<bool> mOWOPConnected;
	std::atomic<bool> mWsConnected;

	virtual void WSMessageHandler(Ws::ConnectionHdl hdl, Ws::MessagePtr msg);
	virtual void WSOpenHandler(Ws::ConnectionHdl hdl);
	virtual void WSCloseHandler(Ws::ConnectionHdl hdl);
	virtual void WSFailHandler(Ws::ConnectionHdl hdl);

	Ws::ConnectionHdl mConnectionHdl;
};