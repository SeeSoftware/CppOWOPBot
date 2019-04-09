#include "TaskManager.h"

void TaskManager::PushTask(const Task & task)
{
	if (task.type == Task::None)
		return;

	std::unique_lock<std::shared_mutex> lock(mMutex);
	
	ContainterType &container = mTasks[task.type];
	container.push_back(task);

}

bool TaskManager::PopTask(Task::Type type, Task & task)
{
	if (type == Task::None)
		return false;

	std::unique_lock<std::shared_mutex> lock(mMutex);

	ContainterType &container = mTasks[type];
	if (container.size() > 0)
	{
		task = container.front();
		container.pop_front();
		return true;
	}

	return false;
}

void TaskManager::ClearTasks(Task::Type type)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	mTasks[type].clear();
}

void TaskManager::ClearTasks()
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	mTasks.clear();
}

void TaskManager::Update(Task::Type type, std::function<void(ContainterType&)> func)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);

	ContainterType &container = mTasks[type];
	func(container);
}

void TaskManager::Update(std::function<void(std::unordered_map<Task::Type, ContainterType>&)> func)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	func(mTasks);
}

size_t TaskManager::GetNumTasks(Task::Type type) const
{
	if (type == Task::None)
		return 0;

	std::shared_lock<std::shared_mutex> lock(mMutex);

	if (mTasks.count(type) == 0)
		return 0;

	return mTasks.at(type).size();
}

size_t TaskManager::GetTotalNumTasks() const
{
	static size_t lastSize = 0;
	if (mMutex.try_lock_shared())
	{
		size_t size = 0;
		for (const auto &x : mTasks)
			size += x.second.size();

		lastSize = size;
		mMutex.unlock_shared();
	}

	return lastSize;
}
