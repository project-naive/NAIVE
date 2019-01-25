#include "ThreadPool.h"

#include <stdexcept>

unsigned all_available = std::thread::hardware_concurrency() - 1;

ThreadPool::ThreadPool():
	available([](){
		unsigned aval = all_available;
		all_available = 0;
		return aval; }()),
	states(new passed_vals[available]{}),
	threads(new std::thread[available]{}) {
	switch (available) {
		case unsigned(-1):
			throw
			std::runtime_error("Cannot detect Hardware Concurrency! Falied to auto initialize thread pool!");
		case 0:
			throw
			std::runtime_error("Only one thread available on hardware! Falied to auto initialize thread pool!");
		default:
			for (unsigned i = 0; i < available; i++) {
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
	threads(new std::thread[available]{}) {
	switch (available) {
		case unsigned(-1) :
			throw
			std::runtime_error("Cannot detect Hardware Concurrency! Falied to auto initialize thread pool!");
		case 0:
			throw
				std::runtime_error("Only one thread available on hardware! Falied to auto initialize thread pool!");
		default:
			for (unsigned i = 0; i < available; i++) {
				std::thread t(deploy_func, &states[i]);
				threads[i] = std::move(t);
				threads[i].detach();
			}
			used = available;
			break;
	}
}

ThreadPool::~ThreadPool() {
	for (unsigned i = 0; i < available; i++) {
		std::unique_lock<std::mutex> lck(states[i].mtx);
		states[i].active = false;
		states[i].finished = false;
		states[i].queue.clear();
		states[i].cv.notify_all();
		states[i].cv.wait(lck, [this, i]() {return this->states[i].ready_exit; });
	}
	delete[] threads;
	delete[] states;
}

unsigned ThreadPool::AddThread() {
	std::lock_guard<std::mutex> lck(mtx);
	if(used>=available) return -1;
	unsigned rtn;
	for (unsigned i = 0; i < available; i++) {
		if (!states[i].active&&states[i].ready_exit) {
			states[i].active = true;
			states[i].ready_exit = false;
			states[i].finished = true;
			states[i].num_done = 0;
			std::thread t(deploy_func, &states[i]);
			threads[i] = std::move(t);
			rtn = i;
			break;
		}
	}
	return rtn;
}

bool ThreadPool::RemThread(unsigned ID) {
	std::lock_guard<std::mutex> lck(mtx);
	if(ID>=available) return false;
	{
		std::unique_lock<std::mutex> lck(states[ID].mtx);
		states[ID].active = false;
		states[ID].finished = false;
		states[ID].queue.clear();
		states[ID].cv.notify_all();
	}
	{
		states[ID].mtx.lock();
		states[ID].mtx.unlock();
	}
	states[ID].queue.clear();
	std::thread t;
	threads[ID] = std::move(t);
	used--;
	return true;
}

bool ThreadPool::ThreadTasks(const TaskQueue& added, unsigned thread) {
	if(thread>=available) return false;
	if(states[thread].ready_exit || !states[thread].active) return false;
	states[thread].queue.append(added);
	states[thread].finished = false;
	states[thread].cv.notify_one();
	return true;
}

void ThreadPool::ScheduleTasks(const TaskQueue & added) {
	MasterQueue.append(added);
}

unsigned ThreadPool::QuerieThread(unsigned thread) {
	if (thread >= available) return -1;
	if (states[thread].ready_exit || !states[thread].active) return -1;
	return states[thread].queue.waiting();
}

ThreadPool::TaskQueue::~TaskQueue() {
	mtx.lock();
	delete[] queue;
	mtx.unlock();
}

void ThreadPool::TaskQueue::clear() {
	std::lock_guard<std::mutex> lck(mtx);
	delete[] queue;
	queue = nullptr;
	queue_start = 0;
	queue_end = 0;
	queue_cache = 0;
}

void ThreadPool::TaskQueue::push(const std::function<void()>& function) {
	std::lock_guard<std::mutex> lck(mtx);
	if (( queue_end>queue_start ) && ( queue_end - queue_start == queue_cache )
		|| ( queue_end<queue_start ) && ( queue_start - 1 == queue_end )) {
		size_t new_size = size_t(queue_cache*1.618) + 2;
		std::function<void()>* new_buffer = new std::function<void()>[new_size];
		for (size_t i = queue_start; i <= queue_end || ( queue_end<queue_start && i <= queue_cache ); i++) {
			new_buffer[i - queue_start] = queue[i];
		}
		if (queue_end < queue_start) {
			for (size_t i = 0; i <= queue_end; i++) {
				new_buffer[queue_cache - queue_start + 1 + i] = queue[i];
			}
		}
		std::swap(queue, new_buffer);
		queue_end = queue_cache;
		queue_cache = new_size - 1;
		queue_start = 0;
		delete[] new_buffer;
	}
	if (queue_cache == 0) {
		queue = new std::function<void()>[2]{};
		queue_cache = 1;
	}
	queue[queue_end] = function;
	if (queue_end == queue_cache) queue_end = 0;
	else queue_end++;
	queue[queue_end] = nullptr;
}

std::function<void()> ThreadPool::TaskQueue::pop() {
	std::lock_guard<std::mutex> lck(mtx);
	if (queue_start == queue_end) return nullptr;
	std::function<void()> rtn = queue[queue_start];
	queue[queue_start] = nullptr;
	if (queue_start == queue_cache) queue_start = 0;
	else queue_start++;
	return rtn;
}

bool ThreadPool::TaskQueue::empty() {
	std::lock_guard<std::mutex> lck(mtx);
	return queue_start == queue_end;
}

size_t ThreadPool::TaskQueue::waiting() {
	std::lock_guard<std::mutex> lck(mtx);
	if (queue_start <= queue_end) return queue_end - queue_start;
	else return queue_cache - queue_start + queue_end + 1;
}

void ThreadPool::TaskQueue::append(const TaskQueue & src) {
	std::lock_guard<std::mutex> lck(mtx);
	size_t new_size = queue_cache + src.queue_cache;
	std::function<void()>* new_queue = new std::function<void()>[new_size + 1];
	for (size_t i = queue_start; i < queue_end || ( queue_end < queue_start && i <= queue_cache ); i++) {
		new_queue[i - queue_start] = queue[i];
	}
	if (queue_end < queue_start) {
		for (size_t i = 0; i < queue_end; i++) {
			new_queue[queue_cache - queue_start + 1 + i] = queue[i];
		}
	}
	size_t size1 = ( queue_end >= queue_start ) ? queue_end - queue_start : queue_cache - queue_start + queue_end + 1;
	for (size_t i = src.queue_start; i < src.queue_end || ( src.queue_end < src.queue_start&& i <= src.queue_cache ); i++) {
		new_queue[i - src.queue_start + size1] = src.queue[i];
	}
	if (src.queue_end < src.queue_start) {
		for (size_t i = 0; i < src.queue_end; i++) {
			new_queue[src.queue_cache - src.queue_start + 1 + i + size1] = src.queue[i];
		}
	}
	size_t size2 = ( src.queue_end >= src.queue_start ) ? src.queue_end - src.queue_start : src.queue_cache - src.queue_start + src.queue_end + 1;
	new_queue[size1 + size2] = nullptr;
	std::swap(new_queue, queue);
	delete[] new_queue;
	queue_cache = new_size;
	queue_start = 0;
	queue_end = size1 + size2;
}
