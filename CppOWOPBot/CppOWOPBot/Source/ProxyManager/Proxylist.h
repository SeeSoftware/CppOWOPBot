#pragma once
#include <shared_mutex>
#include <mutex>
#include <deque>
#include <functional>
#include <string>
#include <shared_mutex>
#include <mutex>
#include <atomic>

class Proxylist
{
public:

	Proxylist() = default;

	void LoadProxyfile(const std::string &filename);
	size_t GetNumProxies() const;
	bool GetNextProxy(std::string &proxyIP);
	void SetSettings(uint32_t cursorsPerProxy);
	void ClearProxyList();


	//runs func in a thread safe enviornment
	void Update(std::function<void(std::deque<std::string> &)> func);

private:

	std::atomic<uint32_t> mCursorsPerProxy;
	std::atomic<uint32_t> mCounter = 1;
	mutable std::shared_mutex mMutex;

	std::deque<std::string> mProxylist;

};