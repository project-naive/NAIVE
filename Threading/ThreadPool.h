//A custom thread pool
//Capabilities: syncing and deploying tasks to threads
//Actively checks the state of execution without blocking
//Thread by default do not exit on task finish and waits for new tasks
//Threads have separate pools of tasks, and the pool itself has a
//pool of tasks that can be assigned to available threads on a PollTasks call.

#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <iostream>

extern unsigned all_available;
extern const std::thread::id main_thread_id;

class ThreadPool {
public:
	class TaskQueue {
	private:
		//queue always ends with a nullptr
		std::function<bool()>* queue = nullptr;
		std::atomic<size_t> queue_start = 0;
		//end is the position of the nullptr
		std::atomic<size_t> queue_end = 0;
		//size of queue structure - 1, same as number of available slots
		std::atomic<size_t> queue_cache = 0;
		std::mutex mtx;
	public:
		~TaskQueue();
		void clear();
		void push(const std::function<bool()>& function);
		std::function<bool()> pop();
		bool empty();
		size_t waiting();
		void append(const TaskQueue& src);
	};
	ThreadPool();
	ThreadPool(unsigned num);
	~ThreadPool();
	const unsigned available;
	unsigned GetUsed() const;
	unsigned AddThread();
	bool RemThread(unsigned ID);

	void PollAllTasks();
	void PollTasks();
	void ScheduleTasks(const TaskQueue& added);
	bool ThreadTasks(const TaskQueue& added,unsigned thread);

	bool ThreadWaiting(unsigned thread);
	size_t QuerieSchedule();
	size_t QuerieThread(unsigned thread);
	void WaitAll();
	void WaitThread(unsigned ID);
private:
	std::mutex mtx;
	std::atomic<unsigned> used;
	TaskQueue MasterQueue;
	struct passed_vals {
		std::mutex mtx{};
		std::condition_variable cv{};
		TaskQueue queue{};
		std::atomic<bool> finished = true;
		std::atomic<bool> active = true;
		std::atomic<size_t> num_done = 0;
		std::atomic<bool> ready_exit = false;
	};
	passed_vals* const states;
	std::thread* const threads;
	static void deploy_func(passed_vals* vals);
};

