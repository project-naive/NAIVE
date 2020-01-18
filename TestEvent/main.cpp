#include "component.h"

#include <thread>

class test : public component<test> {
public:
	static int process_specific_event(test* this_,const message& msg, communication& com) {
		if (msg.type_global == EVENT_BASE_LAST) {
			++(this_->counter);
		}
		//			std::cout << "Generic event recieved\n";
		this_->post_event(this_->peer, message{ EVENT_BASE_LAST,0,com.events,com.post });
		return 0;
	};
	void register_incomming(const message& msg, communication& comm) {
#ifndef NDEBUG
		peer = { &comm,comm.id };
#else
		peer = { &comm };
#endif
		comm.callback = process_specific_event;
		initialized = true;
		++con_count;
	}
	void process_noconnect(const message& msg) {};
	test(size_t listen_size = 16) : component(listen_size) {};
	connection_info peer;
	bool initialized = false;
	size_t counter = 0;
	size_t con_count = 0;
};

class test2 : public component<test2> {
public:
	static int process_specific_event(test2* this_, const message& msg, communication& com) {
		if (msg.type_global == EVENT_BASE_LAST) {
			++(this_->counter);
		}
		//			std::cout << "Generic event recieved\n";
		this_->post_event(this_->peer, message{ EVENT_BASE_LAST,0,com.events,com.post });
		return 0;
	};
	void register_incomming(const message& msg, communication& comm) {
#ifndef NDEBUG
		peer = { &comm,comm.id };
#else
		peer = { &comm };
#endif
		initialized = true;
	}
	void process_noconnect(const message& msg) {};
	test2(size_t listen_size = 16) : component(listen_size) {};
	connection_info peer;
	bool initialized = false;
	size_t counter = 0;
};

test* volatile box0=nullptr;
test2* volatile box1=nullptr;
std::thread threads[2];

#include <intrin.h>
#include <iostream>

void thread_func0() {
	box0 = new test();
	test* self = box0;
	while (!box1);
	test2* peer = box1;
	auto t = std::chrono::high_resolution_clock::now();
	uint64_t num = __rdtsc();
//	self->process_events();
	while ((self->con_count) < 2) {
		self->process_events();
		//			if (self->initialized)
		//				break;
	}
	while (uint32_t(test::get_state(self->peer))) {
		self->process_events();
	}
	std::cout << self->counter << '\n';
	delete self;
	box0 = nullptr;
}

void thread_func1() {
	box1 = new test2();
	test2* self = box1;
	while (!box0);
	test* peer = box0;
	connection_info info;
	auto t = std::chrono::high_resolution_clock::now();
	uint64_t num = __rdtsc();
	connection_info temp = self->connect<test>(peer, &(test2::process_specific_event),0, 0);
	info = self->connect<test>(peer, &(test2::process_specific_event), 0, 0);
	self->connect_cancel(temp);
	self->peer = info;
	self->initialized = true;
	while (true) {
		self->process_events();
		if (test::get_state(info) == uint32_t(test::comm_state::COMM_ACKED)
			|| test::get_state(info) == uint32_t(test::comm_state::COMM_EST))
			break;
	}
	self->post_event(self->peer, message{ test::EVENT_BASE_LAST,0,0,0 });
	while (!uint32_t(test::get_state(info))) {
		self->process_events();
	}
	while (__rdtsc() - num < 200000) {
		self->process_events();
	}
	self->disconnect(info);
	while (uint32_t(test::get_state(info))) {
		self->process_events();
	}
	std::cout << self->counter << '\n';
	delete self;
	box1 = nullptr;
}

/*
void thread_func(int index) {
	boxes[index] = new test();
	test* self = boxes[index];
	while (!boxes[int(!bool(index))]);
	test* peer = boxes[int(!bool(index))];
	connection_info info;
	auto t = std::chrono::high_resolution_clock::now();
	uint64_t num = __rdtsc();
	if (index) {
		connection_info temp = self->connect(peer, 0, 0);
		info = self->connect(peer, 0, 0);
		self->connect_cancel(temp);
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
		self->post_event(self->peer, message{ test::EVENT_BASE_LAST,0,0,0 });
		while (__rdtsc()-num<20000000) {
			self->process_events();
		}
		self->disconnect(info);
		while (uint32_t(test::get_state(info))) {
			self->process_events();
		}
	}
	else {
		while (__rdtsc()-num < 50000000) {
			self->process_events();
//			if (self->initialized)
//				break;
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
*/

constexpr size_t size = sizeof(test);
int main() {
	while (true) {
		mi_stats_reset();
		threads[0] = std::move(std::thread(thread_func0));
		threads[1] = std::move(std::thread(thread_func1));
		for (int i = 0; i < 2; ++i) {
			threads[i].join();
		}
		mi_collect(true);
		mi_stats_print(stdout);
	}
//	test test_init(100);
	system("pause");
	return 0;
}

