#include "ThreadPool.h"

#include <stdexcept>

unsigned all_available = std::thread::hardware_concurrency() - 1;
const std::thread::id main_thread_id = std::this_thread::get_id();

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
		states[i].queue.clear();
		states[i].cv.notify_all();
	}
	for (unsigned i = 0; i < available; i++) {
		std::unique_lock<std::mutex> lck(states[i].mtx);
		states[i].cv.wait(lck, [this, i]() {return bool(this->states[i].ready_exit); });
	}
	delete[] threads;
	delete[] states;
}


void ThreadPool::PollTasks() {
	for (unsigned i = 0; !MasterQueue.empty() && i < available && states[i].active; i++) {
		if (states[i].finished) {
			std::unique_lock<std::mutex> lck(states[i].mtx);
			states[i].queue.push(MasterQueue.pop());
			states[i].finished = false;
			states[i].cv.notify_all();
		}
	}
}

void ThreadPool::PollAllTasks() {
	while (!MasterQueue.empty()) {
		PollTasks();
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
		if (!states[i].active&&states[i].ready_exit) {
			states[i].active = true;
			states[i].ready_exit = false;
			states[i].finished = true;
			states[i].num_done = 0;
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
	std::lock_guard<std::mutex> lck(mtx);
	if(ID>=available) return false;
	if(!states[ID].active) return false;
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
	states[thread].queue.append(added);
	states[thread].finished = false;
	states[thread].cv.notify_one();
	return true;
}

void ThreadPool::ScheduleTasks(const TaskQueue & added) {
	MasterQueue.append(added);
}

bool ThreadPool::ThreadWaiting(unsigned thread) {
	return states[thread].active && states[thread].finished;
}

size_t ThreadPool::QuerieThread(unsigned thread) {
	if (thread >= available) return -1;
	if (states[thread].ready_exit || !states[thread].active) return -1;
	return states[thread].queue.waiting();
}

size_t ThreadPool::QuerieSchedule() {
	return MasterQueue.waiting();
}

void ThreadPool::WaitThread(unsigned ID) {
	if (!states[ID].active) return;
	if (states[ID].ready_exit || states[ID].finished) return;
	std::unique_lock<std::mutex> lck(states[ID].mtx);
	states[ID].cv.wait(lck, [this, ID]() {return bool(states[ID].finished); });
}

void ThreadPool::WaitAll() {
	for (unsigned i = 0; i < available; i++) {
		if (std::this_thread::get_id() != threads[i].get_id()) {
			if (!states[i].active) continue;
			if (states[i].ready_exit || states[i].finished) continue;
			std::unique_lock<std::mutex> lck(states[i].mtx);
			states[i].cv.wait(lck,[this, i](){ return bool(states[i].finished);});
		}
	}
}

void ThreadPool::deploy_func(passed_vals * vals) {
	std::unique_lock<std::mutex> lck(vals->mtx);
	while (vals->active) {//adding a condition variable here may be  a good choice
						  //start execution of queue only after the calling thread sets the finished flag to false
		vals->cv.wait(lck, [vals]() { return !vals->finished || !vals->active; });
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

void ThreadPool::TaskQueue::push(const std::function<bool()>& function) {
	std::lock_guard<std::mutex> lck(mtx);
	if (( queue_end>queue_start ) && ( queue_end - queue_start == queue_cache )
		|| ( queue_end<queue_start ) && ( queue_start - 1 == queue_end )) {
		size_t new_size = size_t(queue_cache*1.618) + 2;
		std::function<bool()>* new_buffer = new std::function<bool()>[new_size];
		for (size_t i = queue_start; i <= queue_end || ( queue_end<queue_start && i <= queue_cache ); i++) {
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
}

std::function<bool()> ThreadPool::TaskQueue::pop() {
	std::lock_guard<std::mutex> lck(mtx);
	if (queue_start == queue_end) return nullptr;
	std::function<bool()> rtn = queue[queue_start];
	queue[queue_start] = nullptr;
	if (queue_start == queue_cache) queue_start = 0;
	else queue_start++;
	return rtn;
}

bool ThreadPool::TaskQueue::empty() {
	return queue_start == queue_end;
}

size_t ThreadPool::TaskQueue::waiting() {
	if (queue_start <= queue_end) return queue_end - queue_start;
	else return queue_cache - queue_start + queue_end + 1;
}

void ThreadPool::TaskQueue::append(const TaskQueue & src) {
	std::lock_guard<std::mutex> lck(mtx);
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
		return;
	}
	else {
		size_t new_size = size_t(( size1 + size2 ) * 1.618) + 1;
		std::function<bool()>* new_queue = new std::function<bool()>[new_size + 1];
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
		return;
	}
}
