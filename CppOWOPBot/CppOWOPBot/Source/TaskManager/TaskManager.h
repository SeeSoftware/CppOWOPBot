#pragma once
#include "Task.h"
#include <deque>
#include <unordered_map>
#include <shared_mutex>
#include <mutex>
#include <functional>

class TaskManager
{
public:

	using ContainterType = std::deque<Task>;

	void PushTask(const Task &task);
	bool PopTask(Task::Type type, Task &task);

	void ClearTasks(Task::Type type);
	//clears all tasks
	void ClearTasks();

	//will run func in thread safe enviornment
	void Update(Task::Type type, std::function<void(ContainterType&)> func);
	void Update(std::function<void(std::unordered_map < Task::Type, ContainterType >&)> func);

	size_t GetNumTasks(Task::Type type) const;
	size_t GetTotalNumTasks() const;

private:

	mutable std::shared_mutex mMutex;
	std::unordered_map < Task::Type, ContainterType > mTasks;

};