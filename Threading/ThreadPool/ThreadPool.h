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

unsigned GetThreadIndex(unsigned init_index = -1);

class ThreadPool {
public:
	class TaskQueue {
	private:
		//queue always ends with a nullptr
		//functions return true for rescheduling 
		//if dependent data is not ready after timeout.
		std::atomic<bool> in_use = false;
		std::function<bool()>* queue = nullptr;
		size_t queue_start = 0;
		//end is the position of the nullptr
		size_t queue_end = 0;
		//size of queue structure - 1, same as number of available slots
		size_t queue_cache = 0;
	public:
		~TaskQueue();
		void clear();
		//Currently the lock is implemented with a atomic bool.
		//If exception happens, the boolean is not unlocked and may result in a deadlock
		//that eats up system resource.
		//Non-Exception-Safe!
		void push(const std::function<bool()>& function);
		std::function<bool()> pop();
		void append(const TaskQueue& src);
		//These functions do not use the in_use for spin lock
		//Thus, are prone to error if race condition occurs 
		bool empty();
		bool empty_strict();
		size_t waiting();
		size_t waiting_strict();
	};
	ThreadPool();
	ThreadPool(unsigned num);
	//ThreadPools can only be destroyed on the creation thread.
	~ThreadPool();
	const unsigned available;
	unsigned GetUsed() const;
	unsigned AddThread();
	bool RemThread(unsigned ID);

	//No mutex involved in Poll functions and Schedule functions!
	void PollAllTasks(float timeout = 0);
	void PollTasks();
	void PushTask(const std::function<bool()>& added);
	void ScheduleTasks(const TaskQueue& added);
	void ClearSchedule();
	void ClearAllTasks();

	bool ThreadTasks(const TaskQueue& added,unsigned thread);
	bool ThreadAvailable(unsigned thread);
	size_t ThreadTODO(unsigned thread);
	size_t QuerieSchedule();
	size_t QuerieThread(unsigned thread);
	unsigned WaitAll(float timeout = 0);
	bool WaitThread(unsigned ID, float timeout = 0);
private:
	std::mutex mtx;
	std::atomic<unsigned> used;
	TaskQueue MasterQueue;
	struct passed_vals {
		std::mutex mtx{};
		std::condition_variable cv{};
		TaskQueue queue{};
		bool waiting = false;
		bool active = true;
		bool finished = true;
		size_t num_done = 0;
		bool ready_exit = false;
		//The structure is exclusive to the executing thread and values 
		//are indicators for current execution state,
		//with the exception of the pool adding things from other thread.
		//setting this can allow a spin-lock when doing light-weight job.
		//Lock both this and the mutex for long-running dispatchs
		//Light-weight and fast changes should use this indicator instead.
		std::atomic<bool> modifying = false;
		unsigned index;
	};
	passed_vals* const states;
	std::thread* const threads;
	static void deploy_func(passed_vals* vals);
	const std::thread::id creation_thread;
};

