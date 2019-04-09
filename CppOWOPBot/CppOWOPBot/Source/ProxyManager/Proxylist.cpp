#include "Proxylist.h"
#include <fstream>

void Proxylist::LoadProxyfile(const std::string & filename)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);

	std::fstream file(filename);
	std::string line;
	while (std::getline(file, line))
	{
		mProxylist.push_back(line);
	}
}

size_t Proxylist::GetNumProxies() const
{
	std::shared_lock<std::shared_mutex> lock(mMutex);
	return mProxylist.size();
}

bool Proxylist::GetNextProxy(std::string & proxyIP)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);

	if (mProxylist.size() == 0)
		return false;

	proxyIP = mProxylist.front();

	if (mCounter % mCursorsPerProxy == 0)
		mProxylist.pop_front();
		
	mCounter++;

	return true;
}

void Proxylist::SetSettings(uint32_t cursorsPerProxy)
{
	mCounter = 1;
	mCursorsPerProxy = cursorsPerProxy;
}

void Proxylist::ClearProxyList()
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	mProxylist.clear();
}

void Proxylist::Update(std::function<void(std::deque<std::string>&)> func)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	func(mProxylist);
}
