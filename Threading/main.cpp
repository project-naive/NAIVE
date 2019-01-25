#include <iostream>

#include "ThreadPool.h"

std::mutex cout_mtx;

void test() {
//	std::unique_lock<std::mutex> lck(cout_mtx);
	std::cout << "Executing dispatched function!\n"
		<< "Hello from thread " << std::this_thread::get_id() << '\n' << std::endl;
}

int main() {
	{
		ThreadPool pool1(2);
		ThreadPool::TaskQueue queue1;
		queue1.push(test);
		queue1.push(test);
		queue1.push(test);
		queue1.pop();
		queue1.pop();
		queue1.pop();
		queue1.push(test);
		queue1.push(test);
		queue1.push(test);
		queue1.pop();
		queue1.pop();
		queue1.push(test);
		ThreadPool::TaskQueue queue2;
		queue2.push(test);
		queue2.push(test);
		queue2.push(test);
		queue2.pop();
		queue2.pop();
		queue2.pop();
		queue2.push(test);
		queue2.push(test);
		queue2.push(test);
		queue2.pop();
		queue2.pop();
		queue2.push(test);
		queue1.append(queue2);
		pool1.ScheduleTasks(queue1);
		ThreadPool pool2;
		pool2.ScheduleTasks(queue2);
		while (pool1.QuerieSchedule()||pool2.QuerieSchedule()) {
			pool1.PollTasks();
			pool2.PollTasks();
		}

	}
	system("pause");
}

