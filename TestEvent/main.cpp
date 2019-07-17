#include "../Engine/eventloop.h"

#include <thread>

test** boxes;
std::thread* threads;

void thread_func(int index) {
	boxes[index] = new test();
	test* peer = boxes[int(!bool(index))];
	peer->connect(boxes[index]);
	delete boxes[index];
}

int main() {
	boxes = new test*[2];
	threads = new std::thread[2];
	for (int i = 0; i < 2; ++i) {
//		threads[i].join();
//		threads[i] = std::move(std::thread(thread_func, i));
	}
	test test_init(100);
	system("pause");
	return 0;
}

