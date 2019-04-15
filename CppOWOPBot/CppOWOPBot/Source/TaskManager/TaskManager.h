#pragma once
#include "Task.h"
#include <deque>
#include <unordered_map>
#include <shared_mutex>
#include <mutex>
#include <functional>


class UnsafeTaskManager
{
public:

	using ContainterType = std::deque<Task>;

	void PushTask(Task task);
	void PushTaskFront(Task task);
	bool PopTask(Task::Type type, Task &task);

	void ClearTasks(Task::Type type);
	//clears all tasks
	void ClearTasks();

	std::unordered_map < Task::Type, ContainterType > &GetTasks();

	size_t GetNumTasks(Task::Type type) const;
	size_t GetTotalNumTasks() const;

private:
	std::unordered_map < Task::Type, ContainterType > mTasks;

};


class TaskManager
{
public:

	using ContainterType = std::deque<Task>;

	void PushTask(Task task);
	void PushTaskFront(Task task);
	bool PopTask(Task::Type type, Task &task);

	void ClearTasks(Task::Type type);
	//clears all tasks
	void ClearTasks();

	size_t GetNumTasks(Task::Type type) const;
	size_t GetTotalNumTasks() const;

	//will run func in thread safe enviornment
	void Update(std::function<void(UnsafeTaskManager&)> func);

private:

	mutable std::shared_mutex mMutex;
	UnsafeTaskManager mTaskManager;
};