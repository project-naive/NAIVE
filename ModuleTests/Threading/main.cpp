#include <iostream>

#include "ThreadPool\ThreadPool.h"

std::mutex cout_mtx;
std::atomic<size_t> times = 0;

size_t counts[7]{};

bool test(ThreadPool* pool, int push_ID) {
/*
	{
//	std::lock_guard<std::mutex> lck(cout_mtx);
	std::cout << "Executing dispatched function!\nCount of execution: " << ++times
		<< "\nTask ID: " << push_ID
		<< "\nThread Index: " << GetThreadIndex()
		<< "\nPool Index: " << GetPoolIndex()
		<< "\nTask execution count: " << ++counts[push_ID]
		<< "\nHello from thread " << std::this_thread::get_id() << '\n' << std::endl;
	}
*/
	pool->PushTask(std::bind(*test, pool, push_ID));
//	pool->PollTasks();
	return false;
}

bool test2(ThreadPool* pool, int push_ID) {
	std::lock_guard<std::mutex> lck(cout_mtx);
	std::cout << "Executing dispatched function!\nCount of execution: " << ++times
		<< "\nTask ID: " << push_ID
		<< "\nTask execution count: " << ++counts[push_ID]
		<< "\nHello from thread " << std::this_thread::get_id() << '\n' << std::endl;
	return false;
}


int main() {
//	std::srand(std::chrono::duration_cast<std::chrono::duration<unsigned>>(std::chrono::steady_clock::now().time_since_epoch()).count());
	try{
		ThreadPool pool;
		ThreadPool::TaskQueue queue;
		queue.push(std::bind(test, &pool, 0));
		queue.push(std::bind(test, &pool, 1));
		queue.push(std::bind(test, &pool, 2));
		queue.push(std::bind(test, &pool, 3));
		queue.push(std::bind(test, &pool, 4));
		queue.push(std::bind(test, &pool, 5));
		queue.push(std::bind(test, &pool, 6));
		pool.ScheduleTasks(queue);
		do{
			while (pool.QuerieSchedule()) {
				pool.PollAllTasks_WaitingOnly();
				pool.WaitAll();
			}
		} while (pool.QuerieSchedule());
		system("pause");
	} catch (std::exception& e) {
		std::cout<<e.what();
	}
	system("pause");
	return 0;
}

