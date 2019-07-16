#pragma once

#include <cstdint>
#include <rigtorp/SPSCQueue.h>
#include <mpsc/mpsc_queue.h>

class eventbox {
	struct message {
		uint64_t type;
		void* msg;
	};
	static rigtorp::SPSCQueue<message>** msg_boxes;
	static thread_local rigtorp::SPSCQueue<message>* this_thread;
	static thread_local bool registered;
	static MPSCQueue<rigtorp::SPSCQueue<message>*> register_queue;
public:
	void register_thread() {
		registered = true;
		register_queue.Push(new MPSCQueue<rigtorp::SPSCQueue<message>*>::Entry{ nullptr,this_thread });
	};
	void push(message m) {
		this_thread->push(m);
		if (!registered) register_thread();
	};
	void push_registered(message m) {
		this_thread->push(m);
	};
	virtual void process_single_event(const message& m) = 0;
	void poll_events() {
		for()
	};
};
