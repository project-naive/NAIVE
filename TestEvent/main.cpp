#include "../Engine/eventbox.h"

#include <thread>

test* boxes[2]{};
std::thread threads[2];

#include <intrin.h>

void thread_func(int index) {
	boxes[index] = new test();
	test* self = boxes[index];
	while (!boxes[int(!bool(index))]);
	test* peer = boxes[int(!bool(index))];
	test::connection_info info;
	auto t = std::chrono::high_resolution_clock::now();
	uint64_t num = __rdtsc();
	if (index) {
		info = self->connect(peer, 0, 0);
		self->peer = info;
		self->initialized = true;
//		self->connect_cancel(info);
	}
//	std::this_thread::sleep_for(std::chrono::duration<double>(1));
//	self->process_events();
//	std::this_thread::sleep_for(std::chrono::duration<double>(1));
	self->process_events();
	if (index) {
		while (true) {
			self->process_events();
			if (test::get_state(info) == uint32_t(test::comm_state::COMM_ACKED)
				|| test::get_state(info) == uint32_t(test::comm_state::COMM_EST))
				break;
		}
		self->post_event(self->peer, test::message{ test::EVENT_BASE_LAST,0,0,0 });
		while (__rdtsc() - num < 1000000000) {
			self->process_events();
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
	std::cout << "Generic event processed:" << self->counter << '\n';
	std::cout <<"Total time:"<< __rdtsc() - num << '\n';
	delete boxes[index];
	boxes[index] = nullptr;
}

int main() {
	while (true) {
		mi_stats_reset();
		for (int i = 0; i < 2; ++i) {
			threads[i] = std::move(std::thread(thread_func, i));
		}
		for (int i = 0; i < 2; ++i) {
			threads[i].join();
		}
//		mi_collect(true);
		mi_stats_print(stdout);
	}
//	test test_init(100);
	system("pause");
	return 0;
}

