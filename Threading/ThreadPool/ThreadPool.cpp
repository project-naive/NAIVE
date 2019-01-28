#include "ThreadPool.h"

#include <stdexcept>

unsigned all_available = std::thread::hardware_concurrency() - 1;

ThreadPool::ThreadPool():
	available([](){
		unsigned aval = all_available;
		all_available = 0;
		return aval; }()),
	states(new passed_vals[available]{}),
	threads(new std::thread[available]{}),
	creation_thread(std::this_thread::get_id()){
	switch (available) {
		case unsigned(-1):
			throw
			std::runtime_error("Cannot detect Hardware Concurrency! Falied to auto initialize thread pool!");
		case 0:
			throw
			std::runtime_error("Only one thread available on hardware! Falied to auto initialize thread pool!");
		default:
			for (unsigned i = 0; i < available; i++) {
				states[i].index = i;
				std::thread t(deploy_func, &states[i]);
				threads[i] = std::move(t);
				threads[i].detach();
			}
			used = available;
			break;
	}
}

ThreadPool::ThreadPool(unsigned num):
	available([num](){
	if(num<=all_available) {all_available-=num; return num;} else throw std::runtime_error("Not Enough hardware for thread pool creation!");}()),
	states(new passed_vals[available]{}),
	threads(new std::thread[available]{}),
	creation_thread(std::this_thread::get_id()) {
	switch (available) {
		case unsigned(-1) :
			throw
			std::runtime_error("Cannot detect Hardware Concurrency! Falied to auto initialize thread pool!");
		case 0:
			throw
				std::runtime_error("Only one thread available on hardware! Falied to auto initialize thread pool!");
		default:
			for (unsigned i = 0; i < available; i++) {
				states[i].index = i;
				std::thread t(deploy_func, &states[i]);
				threads[i] = std::move(t);
				threads[i].detach();
			}
			used = available;
			break;
	}
}

ThreadPool::~ThreadPool() {
	unsigned Thread_Index = GetThreadIndex();
	for (unsigned i = 0; i < available; i++) {
		states[i].active = false;
		states[i].queue.clear();
		if (i != Thread_Index) {
			while(states[i].modifying.exchange(true, std::memory_order_acq_rel));
			while (!states[i].waiting.load(std::memory_order_acquire));
			std::unique_lock<std::mutex> lck(states[i].mtx);
			states[i].cv.notify_all();
		}
	}
	for (unsigned i = 0; i < available; i++) {
		if (i != Thread_Index) {
			std::unique_lock<std::mutex> lck(states[i].mtx);
			states[i].cv.wait(lck, [this, i]() {return bool(this->states[i].ready_exit); });
		}
	}
	delete[] threads;
	delete[] states;
}


void ThreadPool::PollTasks() {
	unsigned ThisThread = GetThreadIndex();
	for (unsigned i = 0; !MasterQueue.empty() && i < available ; i++) {
		if (states[i].active && states[i].queue.empty() && !states[i].modifying.exchange(true, std::memory_order_acq_rel)) {
			try{
				states[i].queue.push(MasterQueue.pop());
				states[i].modifying.store(false, std::memory_order_release);
			} catch (std::exception& e) {
				states[i].modifying.store(false, std::memory_order_release);
				throw e;
			} catch (...) {
				states[i].modifying.store(false, std::memory_order_release);
				throw;
			}
		}
	}
	for (unsigned i = 0; !MasterQueue.empty() && i < available; i++) {
		if (i != ThisThread && states[i].active && states[i].finished && !states[i].queue.empty() && !states[i].modifying.exchange(true, std::memory_order_acq_rel)) {
			while (!states[i].waiting.load(std::memory_order_acquire));
			states[i].cv.notify_all();
			states[i].modifying.store(false, std::memory_order_release);
		}
	}
}

void ThreadPool::PollTasks_WaitingOnly() {
	for (unsigned i = 0; !MasterQueue.empty() && i < available; i++) {
		if (states[i].active && states[i].finished  && states[i].queue.empty() && !states[i].modifying.exchange(true, std::memory_order_acq_rel)) {
			try {
				states[i].queue.push(MasterQueue.pop());
				states[i].modifying.store(false, std::memory_order_release);
			} catch (std::exception& e) {
				states[i].modifying.store(false, std::memory_order_release);
				throw e;
			} catch (...) {
				states[i].modifying.store(false, std::memory_order_release);
				throw;
			}
		}
	}
	unsigned ThisThread = GetThreadIndex();
	for (unsigned i = 0; !MasterQueue.empty() && i < available; i++) {
		if (i != ThisThread && states[i].finished && states[i].active && !states[i].queue.empty() && !states[i].modifying.exchange(true, std::memory_order_acq_rel)) {
			while (!states[i].waiting.load(std::memory_order_acquire));
			states[i].cv.notify_all();
			states[i].modifying.store(false, std::memory_order_release);
		}
	}
}

void ThreadPool::PushTask(const std::function<bool()>& added) {
	MasterQueue.push(added);
}

void ThreadPool::PollAllTasks(float timeout) {
	if (!timeout) {
		while (!MasterQueue.empty()) {
			PollTasks();
		}
	} else {
		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
		while (!MasterQueue.empty() && timeout < std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::steady_clock::now()-start).count()) {
			PollTasks();
		}
	}
}

void ThreadPool::PollAllTasks_WaitingOnly(float timeout) {
	if (!timeout) {
		while (!MasterQueue.empty()) {
			PollTasks_WaitingOnly();
		}
	} else {
		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
		while (!MasterQueue.empty() && timeout < std::chrono::duration_cast<std::chrono::duration<float>>( std::chrono::steady_clock::now() - start ).count()) {
			PollTasks_WaitingOnly();
		}
	}
}

unsigned ThreadPool::GetUsed() const {
	return used;
}

unsigned ThreadPool::AddThread() {
	std::lock_guard<std::mutex> lck(mtx);
	if(used>=available) return -1;
	unsigned rtn;
	for (unsigned i = 0; i < available; i++) {
		if (!states[i].active && states[i].ready_exit) {
			states[i].active = true;
			states[i].ready_exit = false;
			states[i].waiting.store(false, std::memory_order_release);
			states[i].num_done = 0;
			states[i].modifying.store(false, std::memory_order_release);
			std::thread t(deploy_func, &states[i]);
			threads[i] = std::move(t);
			threads[i].detach();
			rtn = i;
			used++;
			break;
		}
	}
	return rtn;
}

bool ThreadPool::RemThread(unsigned ID) {
	if (ID >= available) return false;
	if (!states[ID].active) return false;
	std::lock_guard<std::mutex> lck(mtx);
	if(ID==GetThreadIndex()){ 
		states[ID].active = false;
		return true;
	}
	while(states[ID].modifying.exchange(true, std::memory_order_acq_rel));
	std::unique_lock<std::mutex> thread_lck(states[ID].mtx);
	states[ID].active = false;
	states[ID].queue.clear();
	states[ID].cv.notify_all();
	states[ID].cv.wait(thread_lck, [this, ID]() {return bool(this->states[ID].ready_exit);});
	states[ID].queue.clear();
	std::thread t;
	threads[ID] = std::move(t);
	used--;
	return true;
}

bool ThreadPool::ThreadTasks(const TaskQueue& added, unsigned thread) {
	if(thread>=available) return false;
	if(states[thread].ready_exit || !states[thread].active) return false;
	while(states[thread].modifying.exchange(true, std::memory_order_acq_rel));
	try{
		states[thread].queue.append(added);
		states[thread].cv.notify_all();
		states[thread].modifying.store(false, std::memory_order_release);
		return true;
	} catch (std::exception& e) {
		states[thread].modifying.store(false, std::memory_order_release);
		throw e;
	} catch (...) {
		states[thread].modifying.store(false, std::memory_order_release);
		throw;
	}
}

bool ThreadPool::ThreadPush(const std::function<bool()>& added, unsigned thread) {
	if (thread >= available) return false;
	if (states[thread].ready_exit || !states[thread].active) return false;
	while (states[thread].modifying.exchange(true, std::memory_order_acq_rel));
	try {
		states[thread].queue.push(added);
		states[thread].cv.notify_all();
		states[thread].modifying.store(false, std::memory_order_release);
		return true;
	} catch (std::exception& e) {
		states[thread].modifying.store(false, std::memory_order_release);
		throw e;
	} catch (...) {
		states[thread].modifying.store(false, std::memory_order_release);
		throw;
	}
}

void ThreadPool::ScheduleTasks(const TaskQueue & added) {
	MasterQueue.append(added);
}

void ThreadPool::ClearSchedule() {
	MasterQueue.clear();
}

void ThreadPool::ClearAllTasks() {
	unsigned Thread_Index = GetThreadIndex();
	for (unsigned i = 0; i < available; i++) {
		if (i != Thread_Index) {
			while (states[i].modifying.exchange(true, std::memory_order_acq_rel));
				std::unique_lock<std::mutex> lck(states[i].mtx);
		}
		states[i].queue.clear();
		states[i].cv.notify_all();
		states[i].modifying.store(false, std::memory_order_release);
	}
}

bool ThreadPool::ThreadAvailable(unsigned thread) {
	if (thread >= available) return false;
	if (states[thread].ready_exit || !states[thread].active) return false;
	return states[thread].active && states[thread].waiting.load(std::memory_order_acquire);
}

size_t ThreadPool::QuerieThread(unsigned thread) {
	if (thread >= available) return -1;
	if (states[thread].ready_exit || !states[thread].active) return -1;
	return states[thread].queue.waiting();
}

size_t ThreadPool::QuerieSchedule() {
	return MasterQueue.waiting();
}

bool ThreadPool::WaitThread(unsigned ID, float timeout) {
	if (ID >= available) return false;
	if (!states[ID].active) return false;
	if (states[ID].ready_exit || states[ID].queue.empty()) return true;
	if(ID==GetThreadIndex()) return false;
	std::unique_lock<std::mutex> lck(states[ID].mtx);
	if (!timeout) {
		states[ID].cv.wait(lck, [this, ID]() {return bool(states[ID].queue.empty()); });
		return true;
	}
	else {
		return states[ID].cv.wait_for(lck, std::chrono::duration<float>(timeout), [this, ID]() {return bool(states[ID].queue.empty()); });
	}
}


bool ThreadPool::WaitThreadBlock(unsigned ID, float timeout) {
	if (ID >= available) return false;
	if (!states[ID].active) return false;
	if (states[ID].ready_exit || states[ID].queue.empty()) return true;
	if (ID == GetThreadIndex()) return false;
	while (states[ID].modifying.exchange(true, std::memory_order_acq_rel));
	std::unique_lock<std::mutex> lck(states[ID].mtx);
	if (!timeout) {
		try {
			states[ID].cv.wait(lck, [this, ID]() {return bool(states[ID].queue.empty()); });
		} catch (std::exception& e) {
			states[ID].modifying.store(false, std::memory_order_release);
			throw e;
		} catch (...) {
			states[ID].modifying.store(false, std::memory_order_release);
			throw;
		}
		states[ID].modifying.store(false, std::memory_order_release);
		return true;
	}
	else {
		bool rtn;
		try {
			rtn = states[ID].cv.wait_for(lck, std::chrono::duration<float>(timeout), [this, ID]() {return bool(states[ID].queue.empty()); });
		} catch (std::exception& e) {
			states[ID].modifying.store(false, std::memory_order_release);
			throw e;
		} catch (...) {
			states[ID].modifying.store(false, std::memory_order_release);
			throw;
		}
		states[ID].modifying.store(false, std::memory_order_release);
		return rtn;
	}
}

unsigned ThreadPool::WaitAllBlock(float timeout) {
	unsigned rtn = 0;
	unsigned ThreadIndex = GetThreadIndex();
	if (!timeout) {
		for (unsigned i = 0; i < available; i++) {
			if (i != ThreadIndex) {
				if (!states[i].active) continue;
				if (states[i].ready_exit || states[i].finished) continue;
				while(states[i].modifying.exchange(true, std::memory_order_acq_rel));
				try {
					std::unique_lock<std::mutex> lck(states[i].mtx);
					states[i].cv.wait(lck, [this, i]() { return bool(states[i].finished); });
					states[i].modifying.store(false, std::memory_order_release);
				} catch (std::exception& e) {
					states[i].modifying.store(false, std::memory_order_release);
					throw e;
				} catch (...) {
					states[i].modifying.store(false, std::memory_order_release);
					throw;
				}
				rtn++;
			}
		}
		return rtn;
	} else {
		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
		float time_passed = 0;
		while (timeout > time_passed) {
			for (unsigned i = 0; i < available && timeout > time_passed; i++) {
				if (i != ThreadIndex) {
					if (!states[i].active) continue;
					if (states[i].ready_exit || states[i].queue.empty()) continue;
					while(states[i].modifying.exchange(true, std::memory_order_acq_rel));
					try {
						std::unique_lock<std::mutex> lck(states[i].mtx);
						rtn += states[i].cv.wait_for(lck, std::chrono::duration<float>(timeout - time_passed), [this, i]() { return bool(states[i].queue.empty()); });
					} catch (std::exception& e) {
						states[i].modifying.store(false, std::memory_order_release);
						throw e;
					} catch (...) {
						states[i].modifying.store(false, std::memory_order_release);
						throw;
					}
					states[i].modifying.store(false, std::memory_order_release);
					time_passed = std::chrono::duration_cast<std::chrono::duration<float>>( std::chrono::steady_clock::now() - start ).count();
				}
			}
			time_passed = std::chrono::duration_cast<std::chrono::duration<float>>( std::chrono::steady_clock::now() - start ).count();
		}
		return rtn;
	}
}

unsigned ThreadPool::WaitAll(float timeout) {
	unsigned rtn = 0;
	unsigned ThreadIndex = GetThreadIndex();
	if (!timeout) {
		for (unsigned i = 0; i < available; i++) {
			if (i != ThreadIndex) {
				if (!states[i].active) continue;
				if (states[i].ready_exit || states[i].finished) continue;
				std::unique_lock<std::mutex> lck(states[i].mtx);
				states[i].cv.wait(lck, [this, i]() { return bool(states[i].finished); });
				rtn++;
			}
		}
		return rtn;
	}
	else {
		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
		float time_passed = 0;
		while (timeout > time_passed) {
			for (unsigned i = 0; i < available && timeout > time_passed; i++) {
				if (i != ThreadIndex) {
					if (!states[i].active) continue;
					if (states[i].ready_exit || states[i].queue.empty()) continue;
					std::unique_lock<std::mutex> lck(states[i].mtx);
					rtn += states[i].cv.wait_for(lck, std::chrono::duration<float>(timeout - time_passed), [this, i]() { return bool(states[i].queue.empty()); });
					time_passed = std::chrono::duration_cast<std::chrono::duration<float>>( std::chrono::steady_clock::now() - start ).count();
				}
			}
			time_passed = std::chrono::duration_cast<std::chrono::duration<float>>( std::chrono::steady_clock::now() - start ).count();
		}
		return rtn;
	}
}

void ThreadPool::deploy_func(passed_vals* vals) {
	GetThreadIndex(vals->index);
	std::unique_lock<std::mutex> lck(vals->mtx);
	while (vals->active) {//adding a condition variable here may be  a good choice
						  //start execution of queue only after the calling thread sets the finished flag to false	
		vals->waiting.store(true, std::memory_order_release);
		while (vals->queue.empty() && vals->active) {
			//if a notification comes here, it may be missed!
			vals->cv.wait(lck);
//			while (vals->modifying.load(std::memory_order_acquire));
		}
		vals->finished = false;
		vals->waiting.store(false, std::memory_order_release);
		while (vals->active && !vals->queue.empty()) {
			auto func = vals->queue.pop();
			if (func()) {
				vals->queue.push(func);
			}
			vals->num_done++;
		}
		vals->finished = true;
		if (vals->active) vals->cv.notify_all();
	}
	std::notify_all_at_thread_exit(vals->cv, std::move(lck));
	vals->ready_exit = true;
}

ThreadPool::TaskQueue::~TaskQueue() {
	while (in_use.exchange(true, std::memory_order_acq_rel));
	delete[] queue;
}

void ThreadPool::TaskQueue::clear() {
	while (in_use.exchange(true, std::memory_order_acq_rel));
	delete[] queue;
	queue = nullptr;
	queue_start = 0;
	queue_end = 0;
	queue_cache = 0;
	in_use.store(false, std::memory_order_release);
}

void ThreadPool::TaskQueue::push(const std::function<bool()>& function) {
	while (in_use.exchange(true, std::memory_order_acq_rel));
	try{
		if (( queue_end > queue_start ) && ( queue_end - queue_start == queue_cache )
			|| ( queue_end < queue_start ) && ( queue_start - 1 == queue_end )) {
			size_t new_size = size_t(queue_cache*1.618) + 2;
			std::function<bool()>* new_buffer = new std::function<bool()>[new_size] {};
			for (size_t i = queue_start; i <= queue_end || ( queue_end < queue_start && i <= queue_cache ); i++) {
				new_buffer[i - queue_start] = queue[i];
			}
			if (queue_end < queue_start) {
				for (size_t i = 0; i <= queue_end; i++) {
					new_buffer[queue_cache - queue_start + 1 + i] = queue[i];
				}
			}
			std::swap(queue, new_buffer);
			size_t temp = queue_cache;
			queue_end = temp;
			queue_cache = new_size - 1;
			queue_start = 0;
			delete[] new_buffer;
		}
		if (queue_cache == 0) {
			queue = new std::function<bool()>[2]{};
			queue_cache = 1;
		}
		queue[queue_end] = function;
		if (queue_end == queue_cache) queue_end = 0;
		else queue_end++;
		queue[queue_end] = nullptr;
	} catch (std::exception& e) {
		in_use.store(false, std::memory_order_release);
		throw e;
	} catch (...) {
		in_use.store(false, std::memory_order_release);
		throw;
	}
	in_use.store(false, std::memory_order_release);
}

std::function<bool()> ThreadPool::TaskQueue::pop() {
	while (in_use.exchange(true, std::memory_order_acq_rel));
	try{
		if (queue_start == queue_end) return nullptr;
		std::function<bool()> rtn = queue[queue_start];
		queue[queue_start] = nullptr;
		if (queue_start == queue_cache) queue_start = 0;
		else queue_start++;
		in_use.store(false, std::memory_order_release);
		return rtn;
	} catch (std::exception& e) {
		in_use.store(false, std::memory_order_release);
		throw e;
	} catch (...) {
		in_use.store(false, std::memory_order_release);
		throw;
	}
}

bool ThreadPool::TaskQueue::empty() {
	return queue_end == queue_start;
}

bool ThreadPool::TaskQueue::empty_strict() {
	while (in_use.exchange(true, std::memory_order_acq_rel));
	bool rtn = ( queue_end == queue_start );
	in_use.store(false, std::memory_order_release);
	return rtn;
}

size_t ThreadPool::TaskQueue::waiting() {
	if (queue_end >= queue_start) return queue_end - queue_start;
	else return queue_cache - queue_start + queue_end + 1;
}

size_t ThreadPool::TaskQueue::waiting_strict() {
	size_t rtn;
	while (in_use.exchange(true, std::memory_order_acq_rel));
	if (queue_end >= queue_start) rtn = queue_end - queue_start;
	else rtn = queue_cache - queue_start + queue_end + 1;
	in_use.store(false, std::memory_order_release);
	return rtn;
}

void ThreadPool::TaskQueue::append(const TaskQueue & src) {
	while (in_use.exchange(true, std::memory_order_acq_rel));
	try{
		size_t size1 = ( queue_end >= queue_start ) ? queue_end - queue_start : queue_cache - queue_start + queue_end + 1;
		size_t size2 = ( src.queue_end >= src.queue_start ) ? src.queue_end - src.queue_start : src.queue_cache - src.queue_start + src.queue_end + 1;
		if (size_t(( size1 + size2 ) * 1.618) < queue_cache) {
			for (size_t i = src.queue_start; i < src.queue_end || ( src.queue_end < src.queue_start&&i - src.queue_start <= src.queue_cache ); i++) {
				queue[queue_end] = src.queue[src.queue_start + i];
				if (queue_end == queue_cache) queue_end = 0;
				else queue_end++;
			}
			if (src.queue_end < src.queue_start) {
				for (size_t i = 0; i < src.queue_end; i++) {
					queue[queue_end] = src.queue[i];
					if (queue_end == queue_cache) queue_end = 0;
					else queue_end++;
				}
			}
			queue[queue_end] = nullptr;
			in_use.store(false, std::memory_order_release);
			return;
		} else {
			size_t new_size = size_t(( size1 + size2 ) * 1.618) + 1;
			std::function<bool()>* new_queue = new std::function<bool()>[new_size + 1]{};
			for (size_t i = queue_start; i < queue_end || ( queue_end < queue_start && i <= queue_cache ); i++) {
				new_queue[i - queue_start] = queue[i];
			}
			if (queue_end < queue_start) {
				for (size_t i = 0; i < queue_end; i++) {
					new_queue[queue_cache - queue_start + 1 + i] = queue[i];
				}
			}
			for (size_t i = src.queue_start; i < src.queue_end || ( src.queue_end < src.queue_start&& i <= src.queue_cache ); i++) {
				new_queue[i - src.queue_start + size1] = src.queue[i];
			}
			if (src.queue_end < src.queue_start) {
				for (size_t i = 0; i < src.queue_end; i++) {
					new_queue[src.queue_cache - src.queue_start + 1 + i + size1] = src.queue[i];
				}
			}
			new_queue[size1 + size2] = nullptr;
			std::swap(new_queue, queue);
			delete[] new_queue;
			queue_cache = new_size;
			queue_start = 0;
			queue_end = size1 + size2;
			in_use.store(false, std::memory_order_release);
			return;
		}
	} catch (std::exception& e) {
		in_use.store(false, std::memory_order_release);
		throw e;
	} catch (...) {
		in_use.store(false, std::memory_order_release);
		throw;
	}
}

unsigned GetThreadIndex(unsigned init_index) {
	thread_local static const unsigned index = init_index;
	return index;
}
