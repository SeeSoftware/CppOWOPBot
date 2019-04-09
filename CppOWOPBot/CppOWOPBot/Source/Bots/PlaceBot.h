#pragma once
#include <atomic>
#include <deque>
#include "TaskManager/Task.h"
#include "ConnectionBot.h"

class PlaceBot : public ConnectionBot
{
public:
	PlaceBot(BotManager &manager, bool runWorldHandler = true) : ConnectionBot(manager),mRunWorldHandler(runWorldHandler) {}
	virtual ~PlaceBot() = default;

	//if this bot should run the world handler (chunk updates, tile updates, cursor updates etc)
	//also if it should send chunk requests and other stuff
	void SetRunWorldHandler(bool run) { mRunWorldHandler = run; }

	virtual void Update(float dt) override;
	
protected:
	virtual void MessageHandler(const std::shared_ptr<Protocol::IS2CMessage> &message) override;

private:
	std::atomic<bool> mRunWorldHandler;
};