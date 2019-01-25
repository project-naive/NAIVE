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

#include <iostream>

extern unsigned all_available;

class ThreadPool {
public:
	class TaskQueue {
	private:
		//queue always ends with a nullptr
		std::function<void()>* queue = nullptr;
		size_t queue_start = 0;
		//end is the position of the nullptr
		size_t queue_end = 0;
		//size of queue structure - 1, same as number of available slots
		size_t queue_cache = 0;
		std::mutex mtx;
	public:
		~TaskQueue();
		void clear();
		void push(const std::function<void()>& function);
		std::function<void()> pop();
		bool empty();
		size_t waiting();
		void append(const TaskQueue& src);
	};
	ThreadPool();
	ThreadPool(unsigned num);
	~ThreadPool();
	const unsigned available;
	TaskQueue MasterQueue;
	void PollTasks() {
		for (unsigned i = 0; !MasterQueue.empty() && i < available && states[i].active; i++) {
			if (states[i].finished) {
				std::unique_lock<std::mutex> lck(states[i].mtx);
				states[i].queue.push(MasterQueue.pop());
				states[i].finished = false;
				states[i].cv.notify_one();
			}
		}
	}
	void PollAllTasks() {
		while (!MasterQueue.empty()) {
			PollTasks();
		}
	}
	unsigned GetUsed() {
		return used;
	}
	unsigned AddThread();
	bool RemThread(unsigned ID);
	bool ThreadTasks(const TaskQueue& added,unsigned thread);
	void ScheduleTasks(const TaskQueue& added);
	unsigned QuerieThread(unsigned thread);
	size_t QuerieSchedule() {
		return MasterQueue.waiting();
	}
private:
	std::mutex mtx;
	unsigned used;
	struct passed_vals {
		std::mutex mtx{};
		std::condition_variable cv{};
		TaskQueue queue{};
		bool finished = true;
		bool active = true;
		size_t num_done{};
		bool ready_exit = false;
	};
	passed_vals* const states;
	std::thread* const threads;
	static void deploy_func(passed_vals* vals) {
		std::unique_lock<std::mutex> lck(vals->mtx);
		while (vals->active) {//adding a condition variable here may be  a good choice
			//start execution of queue only after the calling thread sets the finished flag to false
			vals->cv.wait(lck, [vals](){return !vals->finished;});
			while (vals->active && !vals->queue.empty()) {
				vals->queue.pop()();
				vals->num_done++;
			}
			vals->finished = true;
		}
		vals->ready_exit = true;
		std::notify_all_at_thread_exit(vals->cv,std::move(lck));
	}
};

