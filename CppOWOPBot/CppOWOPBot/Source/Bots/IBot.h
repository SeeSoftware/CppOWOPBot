#pragma once
class BotManager;

class IBot
{
public:
	IBot(BotManager &botManager) :mManager(botManager){}
	virtual ~IBot() = default;

	virtual void Connect(const std::string &uri, const std::string &proxyUri = "") = 0;
	virtual void Disconnect() = 0;
	virtual void Update(float dt) = 0;

protected:
	BotManager &mManager;
};