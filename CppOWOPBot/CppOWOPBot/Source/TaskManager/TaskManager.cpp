#include "TaskManager.h"

void UnsafeTaskManager::PushTask(Task task)
{
	if (task.type == Task::None)
		return;

	ContainterType &container = mTasks[task.type];
	container.push_back(task);

}

void UnsafeTaskManager::PushTaskFront(Task task)
{
	if (task.type == Task::None)
		return;

	ContainterType &container = mTasks[task.type];
	container.push_front(task);
}

bool UnsafeTaskManager::PopTask(Task::Type type, Task & task)
{
	if (type == Task::None)
		return false;

	ContainterType &container = mTasks[type];
	if (container.size() > 0)
	{
		task = container.front();
		container.pop_front();
		return true;
	}

	return false;
}

void UnsafeTaskManager::ClearTasks(Task::Type type)
{
	mTasks[type].clear();
}

void UnsafeTaskManager::ClearTasks()
{
	mTasks.clear();
}

std::unordered_map<Task::Type, UnsafeTaskManager::ContainterType>& UnsafeTaskManager::GetTasks()
{
	return mTasks;
}


size_t UnsafeTaskManager::GetNumTasks(Task::Type type) const
{
	if (type == Task::None)
		return 0;

	if (mTasks.count(type) == 0)
		return 0;

	return mTasks.at(type).size();
}

size_t UnsafeTaskManager::GetTotalNumTasks() const
{
	size_t size = 0;
	for (const auto &x : mTasks)
		size += x.second.size();

	return size;
}



void TaskManager::PushTask(Task task)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	mTaskManager.PushTask(task);
}

void TaskManager::PushTaskFront(Task task)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	mTaskManager.PushTaskFront(task);
}

bool TaskManager::PopTask(Task::Type type, Task & task)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	return mTaskManager.PopTask(type,task);
}

void TaskManager::ClearTasks(Task::Type type)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	mTaskManager.ClearTasks(type);
}

void TaskManager::ClearTasks()
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	mTaskManager.ClearTasks();
}


size_t TaskManager::GetNumTasks(Task::Type type) const
{
	std::shared_lock<std::shared_mutex> lock(mMutex);
	return mTaskManager.GetNumTasks(type);

}

size_t TaskManager::GetTotalNumTasks() const
{
	std::shared_lock<std::shared_mutex> lock(mMutex);
	return mTaskManager.GetTotalNumTasks();
}

void TaskManager::Update(std::function<void(UnsafeTaskManager&)> func)
{
	std::unique_lock<std::shared_mutex> lock(mMutex);
	func(mTaskManager);
}
