#include "../Engine/eventbox.h"

#include <thread>

test* boxes[2];
std::thread threads[2];

#include <intrin.h>

void thread_func(int index) {
	boxes[index] = new test();
	std::this_thread::sleep_for(std::chrono::duration<double>(1));
	test* peer = boxes[int(!bool(index))];
	test* self = boxes[index];
	test::connection_info info;
	auto t = std::chrono::high_resolution_clock::now();
	uint64_t num = __rdtsc();
	if (index) {
		info = self->connect(peer, 0, 0);
		self->peer = info;
//		self->connect_cancel(info);
	}
//	std::this_thread::sleep_for(std::chrono::duration<double>(1));
//	self->process_events();
//	std::this_thread::sleep_for(std::chrono::duration<double>(1));
	self->process_events();
	if (index) {
		while (true) {
			self->process_events();
			if (test::get_state(info) == test::comm_state::COMM_ACKED
				|| test::get_state(info) == test::comm_state::COMM_EST)
				break;
		}
		self->disconnect(info);
		while (uint32_t(test::get_state(info))) {
			self->process_events();
		}
	}
	else {
		while (true) {
			self->process_events();
			if (self->initialized)
				break;
		}
		while (uint32_t(test::get_state(self->peer))) {
			self->process_events();
		}
	}
	std::cout << __rdtsc() - num << '\n';
	delete boxes[index];
}

int main() {
	for (int i = 0; i < 2; ++i) {
		threads[i] = std::move(std::thread(thread_func, i));
	}
	for (int i = 0; i < 2; ++i) {
		threads[i].join();
	}
//	test test_init(100);
	system("pause");
	return 0;
}

