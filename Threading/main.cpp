#include <iostream>

#include "ThreadPool.h"

std::mutex cout_mtx;

bool test(ThreadPool* pool) {
//	std::lock_guard<std::mutex> lck(cout_mtx);
	std::cout << "Executing dispatched function!\n"
		<< "Hello from thread " << std::this_thread::get_id() << '\n' << std::endl;
	ThreadPool::TaskQueue queue;
	queue.push(std::bind(*test, pool));
	pool->ScheduleTasks(queue);
	pool->PollTasks();
	return false;
}

int main() {
	try{
		ThreadPool pool;
		ThreadPool::TaskQueue queue;
		queue.push(std::bind(test,&pool));
		queue.push(std::bind(test, &pool));
		queue.push(std::bind(test, &pool));
		queue.push(std::bind(test, &pool));
		queue.push(std::bind(test, &pool));
		queue.push(std::bind(test, &pool));
		queue.push(std::bind(test, &pool));
		pool.ScheduleTasks(queue);
		pool.PollAllTasks();
		pool.WaitAll();
	} catch (std::exception& e) {
		std::cout<<e.what();
	}
	system("pause");
}

