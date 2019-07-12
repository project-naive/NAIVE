//A custom thread pool
//Capabilities: syncing and deploying tasks to threads
//Actively checks the state of execution without blocking
//Thread by default do not exit on task finish and waits for new tasks
//Threads have separate pools of tasks, and the pool itself has a
//pool of tasks that can be assigned to available threads on a PollTasks call.
//
//Note that a thread will go to wait when the queue gets empty, and waking up from
//such a sleep can be costly at times. It is thus recommended to keep things busy



#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <iostream>

extern std::atomic<int64_t> all_available;


//unsigned(-1) for main thread and threads not managed by a pool
unsigned GetThreadIndex(unsigned init_index = -1);
size_t GetPoolIndex(size_t init_index = -1);


/*
Use ref-count instead of spin-locks for changing allocation
In this way, resource block when there's a need for allocation
The ones that noticed the need for reallocating first get one thread
to actually do the allocation, signalling a spin lock to lock new writing
(currently also locks reading, but may be implemented better by using 
a backup pointer for reading before the copying is done, but syncing 
this will require yet another signalling spin lock)
previous writes will be waited for, but reads are allowed to continue
To allow reads to continue, the reads must back-up the buffer pointers
itself
*/


#include <intrin.h>
class Waitfree_ReallocatingQueue {
typedef std::atomic_size_t ref_count;
typedef std::atomic_bool spin_lock;
typedef std::atomic<std::function<void()>*> pos_ptr;
private:
	//the indicator of a pending or in-progress global change
	//when global change is actually happening, no thread is allowed to access this object
	spin_lock reallocating = false;
	//number of working threads. The global changes must wait for all to finish
	ref_count writing = 0;
	ref_count reading = 0;
	//the total count modulus by 2^31 or 2^63
	//note that the actual stored number is the index fo the next one
	std::atomic_size_t reserve_start = 0;
	std::atomic_size_t reserve_end = 0;
	//indicator of read or write progress for each of the slots
	//reading waits for write to finish (if write needs to reallocate, may block)
	//0 for not ready (the reading thread sets to 0)
	//1 for ready for reading (the writing thread sets to 1)
	std::atomic_bool* slot_ready = nullptr;
	std::function<void()>* buffer = nullptr;
	//guaranteed to be a power of 2 for fast processing
	size_t buff_size = 0;
public:
	//destructor, can only be called when all other threads
	//finishes jobs, otherwise may end up in seg-fault or dead-lock
	~Waitfree_ReallocatingQueue() {
		while (reallocating.exchange(true, std::memory_order_acquire)) _mm_pause();
		while (reading.load(std::memory_order_acquire)) _mm_pause();
		while (writing.load(std::memory_order_acquire)) _mm_pause();
		delete[] buffer;
	}
	//call this function to re-initialize the object, clears all scheduled tasks and also the buffer
	void clear() {
		while (reallocating.exchange(true, std::memory_order_acquire)) _mm_pause();
		while (reading.load(std::memory_order_acquire)) _mm_pause();
		while (writing.load(std::memory_order_acquire)) _mm_pause();
		delete[] buffer;
		buffer = nullptr;
		reserve_start.store(0, std::memory_order_relaxed);
		reserve_end.store(0, std::memory_order_relaxed);
		buff_size = 0;
		reallocating.store(false, std::memory_order_release);
	}
	//quickly checks if the queue is empty, can give false positives
	bool empty() {
		while (reallocating.load(std::memory_order_acquire)) _mm_pause();
		return reserve_end.load(std::memory_order_acquire) <= reserve_start.load(std::memory_order_acquire);
	};
	//stops all pushes and pops to check if the queue is empty at given moment
	bool empty_strict() {
		while (reallocating.exchange(true, std::memory_order_acquire)) _mm_pause();
		while (reading.load(std::memory_order_acquire)) _mm_pause();
		while (writing.load(std::memory_order_acquire)) _mm_pause();
		bool rtn = reserve_start.load(std::memory_order_relaxed) == reserve_end.load(std::memory_order_relaxed);
		reallocating.store(false, std::memory_order_release);
		return rtn;
	};
	//stops all pushes and pops to see content of queue  at given moment
	size_t num_strict() {
		while (reallocating.exchange(true, std::memory_order_acquire)) _mm_pause();
		while (reading.load(std::memory_order_acquire)) _mm_pause();
		while (writing.load(std::memory_order_acquire)) _mm_pause();
		size_t rtn = reserve_end.load(std::memory_order_relaxed) - reserve_start.load(std::memory_order_relaxed);
		reallocating.store(false, std::memory_order_release);
		return rtn;
	};
	//number of threads writing the object, does not add to reader count
	size_t writing() {
		return writing.load(std::memory_order_acq_rel);
	};
	//number of threads reading the object, itself does not count
	size_t reading() {
		return reading.load(std::memory_order_acq_rel);
	};
	void push(const std::function<void()>& function) {
		while (true) {
			size_t id;
			while (true) {
				while (true) {
					writing.fetch_add(1, std::memory_order_acq_rel);
					if (!reallocating.load(std::memory_order_acquire)) break;
					writing.fetch_sub(1, std::memory_order_release);
					while (reallocating.load(std::memory_order_acquire)) _mm_pause();
				}
				id = reserve_end.fetch_add(1, std::memory_order_acq_rel);
#ifdef _M_X64
				if (id < 0x8000000000000000) break;
				if (id == 0x8000000000000000) {
					while (reallocating.exchange(true, std::memory_order_acquire)) _mm_pause();
					reserve_end.fetch_sub(1, std::memory_order_acq_rel);
					writing.fetch_sub(1, std::memory_order_release);
					while (reading.load(std::memory_order_acquire))_mm_pause();
					while (writing.load(std::memory_order_acquire))_mm_pause();
					reserve_end.fetch_sub(0x4000000000000000, std::memory_order_relaxed);
					reserve_start.fetch_sub(0x4000000000000000, std::memory_order_relaxed);
					reallocating.store(false, std::memory_order_release);
					continue;
				}
				if (id > 0x8000000000000000) {
					reserve_end.fetch_sub(1, std::memory_order_acq_rel);
					writing.fetch_sub(1, std::memory_order_release);
					continue;
				}
#else
				if (id < 0x80000000) break;
				if (id == 0x80000000) {
					while (reallocating.exchange(true, std::memory_order_acquire)) _mm_pause();
					reserve_end.fetch_sub(1, std::memory_order_acq_rel);
					writing.fetch_sub(1, std::memory_order_release);
					while (reading.load(std::memory_order_acquire))_mm_pause();
					while (writing.load(std::memory_order_acquire))_mm_pause();
					reserve_end.fetch_sub(0x40000000, std::memory_order_relaxed);
					reserve_start.fetch_sub(0x40000000, std::memory_order_relaxed);
					reallocating.store(false, std::memory_order_release);
					continue;
				}
				if (id > 0x80000000) {
					reserve_end.fetch_sub(1, std::memory_order_acq_rel);
					writing.fetch_sub(1, std::memory_order_release);
					continue;
				}
#endif
			}
			if (id - reserve_start.load(std::memory_order_acquire) < buff_size)break;

		}

	}
	std::function<void()> pop();
};

class ThreadPool {
public:
	//A naive approach for a spin-locked task queue.
	//A better approach may be a lock-free linked list of fixed length arrays or something similar
	//(A pure linked list is not memory-efficient and has too many allocations and deallocations).
	//Or maybe a linked list of stack for "deallocated" nodes from pop could be used to reduce memory
	//allocation time count, this apprach can also add features by putting additional information
	//in the struct alongside the function pointer.
	//
	//Another thought is to implement the queue as a pseudo-fixed-length array queue.
	//When a reallocation of the queue needs to be done, a flag is set to block normal
	//pushing and popping, and start reallocating after all other worker has finished 
	//as indicated by a usage counter. The push and pop also needs a flag to indicate
	//the difference between push or pop being finished and memory being indicated as 
	//under-use by the start and end indicator (all needs to be atomic).
	//(Decided to give up since signalling a overflow of buffer can easily break since
	//the signal time cannot be properly synced in the most general case).
	class TaskQueue {
	private:
		//queue always ends with a nullptr
		//functions return true for rescheduling 
		//if dependent data is not ready after timeout.
		std::atomic_flag in_use = ATOMIC_FLAG_INIT;
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
		//Especially so if the queue is nearly full for empty() function.
		//The idea for error is that false negative is more acceptible 
		//than false positive since pushing and popping has a lock and can
		//never drop a task when pushing, but may pop a nullptr if a
		//false positive occurs, and checking for nullptrs will be bad.
		bool empty();
		//use this to double-check before ending loops.
		bool empty_strict();
		//waiting() function has to be strict when used for evaluation.
		size_t waiting();
	};
	ThreadPool();
	ThreadPool(unsigned num);
	//ThreadPools can only be destroyed on the creation thread.
	~ThreadPool();
	const unsigned available;
	unsigned GetUsed() const;
	unsigned AddThread();
	//This function currently have the bug of a deadlock when
	//two threads are executing it to remove each other.
	bool RemThread(unsigned ID);

	//No mutex involved in Poll functions and Schedule functions!
	void PollAllTasks(float timeout = 0);
	void PollAllTasks_WaitingOnly(float timeout = 0);
	void PollTasks();
	void PollTasks_WaitingOnly();
	void PushTask(const std::function<bool()>& added);
	void ScheduleTasks(const TaskQueue& added);
	void ClearSchedule();
	void ClearAllTasks();

	bool ThreadTasks(const TaskQueue& added, unsigned thread);
	bool ThreadPush(const std::function<bool()>& added, unsigned thread);
	bool ThreadAvailable(unsigned thread);
	size_t QuerieSchedule();
	size_t QuerieThread(unsigned thread);
	unsigned WaitAll(float timeout = 0);
	bool WaitThread(unsigned ID, float timeout = 0);

	void auto_work(unsigned thread, bool work = true);

private:
//	std::mutex mtx;
	std::atomic<unsigned> used;
	TaskQueue MasterQueue;
	struct passed_vals {
		//The structure is exclusive to the executing thread and values 
		//are indicators for current execution state,
		//with the exception of the pool adding things from other thread.
		//setting this can allow a spin-lock when doing light-weight job.
		//Lock both this and the mutex for long-running dispatchs
		//Light-weight and fast changes should use this indicator instead.
		std::atomic_flag modifying = ATOMIC_FLAG_INIT;
		std::atomic<bool> waiting = false;
		std::mutex mtx{};
		std::condition_variable cv{};
		TaskQueue queue{};
		TaskQueue* Masterqueue;
		bool active = true;
		bool finished = true;
		size_t num_done = 0;
		bool ready_exit = false;
		unsigned thread_index;
		size_t pool_index;
		bool auto_get_task = true;
	};
	passed_vals* const states;
	std::thread* const threads;
	static void deploy_func(passed_vals* vals);
	const std::thread::id creation_thread;
	const size_t pool_index;
};

