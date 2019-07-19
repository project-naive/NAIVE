#pragma once

#include <cstdint>
#include <rigtorp/SPSCQueue.h>
#include <rigtorp/MPMCQueue.h>
//#include <mpsc/mpsc_queue.h>
#include <MI/mimalloc.h>

//This class implements a method of lock-free inter-module
//communication. The handle for a communication consists of
//a pointer to target and the target's port number. Since
//most of in-game modules have a relatively long lifetime
//compared to a loop round, the data structure uses a linked
//list of many connections as blocks. If connections were to be
//created and destroyed frequently, a doubly linked list with
//recycling may be better
template<typename T>
class eventbox {
public:
	struct message;
	enum class comm_state: uint32_t {
		COMM_NONE,
		COMM_PENDING,				//applicant sent, no ack
		COMM_ACKED,					//acknowledgement sent, same as est
		COMM_EST,					//communication established
		COMM_ENDWAIT				//unregister message sent
	};
protected:
	struct communication {
		uint32_t state;							//the current state of communication
		uint32_t type;
		rigtorp::SPSCQueue<message>* events;	//the queue to recieve message from
		rigtorp::SPSCQueue<message>* post;		//the queue to send message to
		uint64_t id;
	};
private:
#define COM_SEG ((4096 - 8) / sizeof(eventbox::communication))
	struct alignas(4096) comm_node {
		comm_node* next;
		communication comms[COM_SEG];
	};
	comm_node* head = nullptr;
	size_t num_comm = 0;
	size_t num_used = 0;
	communication** recycled = nullptr;
	size_t num_recycled = 0;
	size_t recycle_size = 0;
	struct queue_block {
		queue_block* next;
		char pad[sizeof(rigtorp::SPSCQueue<message>) - sizeof(queue_block*)];
	};
	queue_block* queue_areas = nullptr;
	rigtorp::MPMCQueue<rigtorp::SPSCQueue<message>*> applications;
	bool recycle_comm_slot(communication* com) {
#ifndef NDEBUG
		assert(com);
#endif
		if (num_recycled == recycle_size) {
			size_t new_size = size_t(recycle_size*1.618) + 1;
			communication** new_place = (communication**)realloc(recycled, new_size * sizeof(communication*));
			if (!new_place) {
				return false;
			}
			recycled = new_place;
			recycle_size = new_size;
		}
		recycled[num_recycled] = com;
		++num_recycled;
		com->id = 0;
		com->state = uint32_t(comm_state::COMM_NONE);
		return true;
	}
	void recycle_queue_block(rigtorp::SPSCQueue<message>* p) {
		if (!queue_areas) {
			queue_areas = (queue_block*)p;
			queue_areas->next = nullptr;
			return;
		}
		((queue_block*)p)->next = queue_areas;
		queue_areas= (queue_block*)p;
	}
	rigtorp::SPSCQueue<message>* assign_queue_block() {
		if (queue_areas) {
			rigtorp::SPSCQueue<message>* ptr = (rigtorp::SPSCQueue<message>*)queue_areas;
			queue_areas = queue_areas->next;
			return ptr;
		}
		return (rigtorp::SPSCQueue<message>*)malloc(sizeof(rigtorp::SPSCQueue<message>));
	}
	void clear_queue_areas(queue_block* node) {
		if (node->next) {
			clear_queue_areas(node->next);
		}
		free(node);
	}
	void remove_recycle(communication* start, communication* end) {
		assert(start);
		assert(end);
		int64_t size = end - start;
		for (size_t i = 0; i < num_recycled; ++i) {
			if (recycled[i] > start && recycled[i] < end) {
				--num_recycled;
				for (size_t j = i; j < num_recycled; ++j) {
					recycled[i] = recycled[i + 1];
				}
			}
		}
	}
	bool clean_comms(comm_node* node, size_t recurse = 0) {
		bool next_okay;
		if (node->next) {
			next_okay = clean_comms(node->next, recurse + 1);
		}
		else { 
			next_okay = true;
		}
		bool okay = next_okay;
		communication* start = nullptr;
		int64_t num_search = 0;
		if (node->next) num_search = COM_SEG - 1;
		else num_search = (num_used - 1) % COM_SEG;
		for (int64_t i = num_search; i >= 0; --i) {
			if (node->comms[i].state) {
				start = &(node->comms[i]);
				okay = false;
				break;
			}
		}
		if (okay) {
			remove_recycle(node->comms - 1, node->comms + COM_SEG);
			num_comm -= COM_SEG;
			mi_free_aligned(node, 4096);
			return okay;
		}
		if (next_okay) {
			remove_recycle(start, node->comms + COM_SEG);
			node->next = nullptr;
			return okay;
		}
	}
protected:
	communication* connect_accept(const message& msg) {
		assert(msg.type_global == EVENT_REGISTER);
		communication* comm = assign_port();
		if (!comm) {
			//deal with failed allocation
			msg.callback->emplace(message{ EVENT_REGISTER_FAIL, msg.type_specific, (rigtorp::SPSCQueue<message>*)this, msg.callback });
			return nullptr;
		}
		*comm = { uint32_t(comm_state::COMM_ACKED), msg.type_specific,(rigtorp::SPSCQueue<message>*)msg.msg , msg.callback };
		comm->post->emplace(message{ EVENT_REGISTER_ACK, msg.type_specific, comm->events, comm->post });
		return comm;
	}
	//authoritative consumer actively unregister producer
	//the producer is responsible for deleting the queue
	bool disconnect_accept(const message& msg, communication& com) {
		assert(msg.type_global == EVENT_UNREGISTER);
		assert(msg.callback == com.post);
		assert(msg.msg == com.events);
		if (!recycle_comm_slot(&com)) {
			com.post->emplace(message{
				EVENT_UNREGISTER_FAIL, msg.type_specific,com.events, com.post });
			return false;
		}
		com.state = uint32_t(comm_state::COMM_NONE);
		com.post->emplace(message{
			EVENT_UNREGISTER_ACK, msg.type_specific,(rigtorp::SPSCQueue<message>*)this, com.post });
		return true;
	}
	void connect_establish(const message& msg, communication& com) {
		assert(msg.callback == com.post);
		assert(msg.msg == com.events);
		assert(com.state == uint32_t(comm_state::COMM_PENDING));
		assert(msg.type_global == EVENT_REGISTER_ACK);
		com.state = uint32_t(comm_state::COMM_EST);
	}
	void connect_accept_cancel(const message& msg, communication& com) {
		assert(com.state == uint32_t(comm_state::COMM_ACKED));
		assert(msg.type_global == EVENT_REGISTER_CANCEL);
		assert(msg.msg == com.events);
		assert(msg.callback == com.post);
		(*com.post).~SPSCQueue<message>();
		recycle_queue_block(com.post);
		(*com.events).~SPSCQueue<message>();
		recycle_queue_block(com.events);
		recycle_comm_slot(&com);
	}
	void disconnect_done(const message& msg, communication& com) {
		assert(msg.type_global == EVENT_UNREGISTER_ACK);
		assert(msg.msg == com.events);
		com.post->~SPSCQueue();
		recycle_queue_block(com.post);
		com.events->~SPSCQueue();
		recycle_queue_block(com.events);
		com.state = uint32_t(comm_state::COMM_NONE);
		recycle_comm_slot(&com);
	}
	void connect_accept_fail(const message& msg, communication& com) {
		assert(msg.msg == com.events);
		assert(com.state == uint32_t(comm_state::COMM_PENDING));
		((eventbox*)msg.callback)->applications.emplace(com.events);
	}
	void disconnect_accept_fail(const message& msg, communication& com) {
		assert(msg.callback == com.post);
		assert(msg.msg == com.events);
		assert(msg.type_global == EVENT_UNREGISTER_FAIL);
		com.post->emplace(message{ EVENT_UNREGISTER, msg.type_specific, com.events,com.post });
	}
	communication* assign_port() {
		if (num_recycled) {
			assert(head);
			--num_recycled;
			communication* ptr = recycled[num_recycled];
			ptr->state = uint32_t(comm_state::COMM_NONE);
			ptr->id = id_max;
			++id_max;
			return ptr;
		}
		if (num_used < num_comm) {
			assert(head);
			comm_node* node = head;
			size_t node_index = num_used / COM_SEG;
			if (node_index) {
				for (size_t i = 0; i < node_index; ++i) {
					assert(node->next);
					node = node->next;
				}
			}
			size_t index = num_used % COM_SEG;
			++num_used;
			communication* ptr = &(node->comms[index]);
			ptr->state = uint32_t(comm_state::COMM_NONE);
			ptr->id = id_max;
			++id_max;
			return ptr;
		}
		comm_node* new_page = (comm_node*)mi_aligned_alloc(4096, 4096);
		if (!new_page) {
			//deal with failed allocation
			return nullptr;
		}
		new_page->next = nullptr;
		comm_node* node = head;
		if (head) {
			while (node->next)
				node = node->next;
			node->next = new_page;
		}
		else 
			head = new_page;
		num_comm += COM_SEG;
		++num_used;
		communication* ptr = &(new_page->comms[0]);
		ptr->state = uint32_t(comm_state::COMM_NONE);
		ptr->id = id_max;
		++id_max;
		return ptr;
	}
//	virtual void process_single_event(const message& msg, communication& com) = 0;
	uint64_t id_max = 1;	//id = 0 means invalid
	int process_single_event(const eventbox::message& msg, communication& com) {
		switch (msg.type_global) {
			case EVENT_REGISTER:
#ifndef NDEBUG
				std::cout << "EVENT_REGISTER GOT.\n";
#endif
				return 0;
			case EVENT_REGISTER_CANCEL:
#ifndef NDEBUG
				std::cout << "EVENT_REGISTER_CANCEL GOT.\n";
#endif
				connect_accept_cancel(msg, com);
				return 2;
			case EVENT_REGISTER_ACK:
#ifndef NDEBUG
				std::cout << "EVENT_REGISTER_ACK GOT.\n";
#endif
				connect_establish(msg, com);
				return 0;
			case EVENT_REGISTER_FAIL:
#ifndef NDEBUG
				std::cout << "EVENT_REGISTER_FAIL GOT.\n";
#endif
				connect_accept_fail(msg, com);
				return 1;
			case EVENT_UNREGISTER:
#ifndef NDEBUG
				std::cout << "EVENT_UNREGISTER GOT.\n";
#endif
				disconnect_accept(msg, com);
				return 2;
			case EVENT_UNREGISTER_CANCEL:
#ifndef NDEBUG
				std::cout << "EVENT_UNREGISTER_CANCEL GOT.\n";
#endif
				abort();
				break;
			case EVENT_UNREGISTER_ACK:
#ifndef NDEBUG
				std::cout << "EVENT_UNREGISTER_ACK GOT.\n";
#endif
				disconnect_done(msg, com);
				return 2;
			case EVENT_UNREGISTER_FAIL:
#ifndef NDEBUG
				std::cout << "EVENT_UNREGISTER_FAIL GOT.\n";
#endif
				disconnect_accept_fail(msg, com);
				return 1;
		}
		return static_cast<T*>(this)->process_specific_event(msg, com);
	};
public:
	enum :uint32_t {
		EVENT_REGISTER,
		EVENT_REGISTER_CANCEL,
		EVENT_REGISTER_ACK,
		EVENT_REGISTER_FAIL,
		EVENT_UNREGISTER,
		EVENT_UNREGISTER_CANCEL,	//impossible
		EVENT_UNREGISTER_ACK,
		EVENT_UNREGISTER_FAIL,
		EVENT_CANNOT_PROCESS_ALL,
		EVENT_REGISTER_PRIORITY,	//not implemented
		EVENT_BASE_LAST
	};
	struct message {
		uint32_t type_global;
		uint32_t type_specific;
		rigtorp::SPSCQueue<message>* callback;
		void* msg;
	};
	struct connection_info {
		void* handle;	//handle of a communication, internally a communication*
#ifndef NDEBUG
		uint64_t id;		//for verification
#endif
	};
	//call this in the producer instance before anything else.
	connection_info connect(eventbox* peer,
		uint32_t self_type, uint32_t peer_type, 
		uint32_t self_events = 100, uint32_t peer_events = 100) {
		communication* comm = assign_port();
#ifndef NDEBUG
		if (!comm) return { nullptr, 0 };
#else
		if (!comm) return { nullptr };
#endif
		comm->state = uint32_t(comm_state::COMM_PENDING);
		comm->events = assign_queue_block();
		if (!comm->events) {
			//failed allocation
		}
		new (comm->events) rigtorp::SPSCQueue<message>(self_events);
		comm->post = assign_queue_block();
		if (!comm->post) {
			//failed allocation
		}
		new (comm->post) rigtorp::SPSCQueue<message>(peer_events);
		comm->type = self_type;
		comm->post->emplace(message{ EVENT_REGISTER, peer_type,comm->events, comm->post });
		//current implementation cannot handle failed registration
		peer->applications.emplace(comm->post);
#ifndef NDEBUG
		return { comm, comm->id };
#else
		return { comm };
#endif
	}
#ifndef NDEBUG
	void verify_connection_info(const connection_info& info) {
		assert(head);
		comm_node* node = head;
		while (node) {
			if ((((communication*)info.handle) - node->comms)>=0 &&
				(((communication*)info.handle) - node->comms) < COM_SEG)
				break;
			node = node->next;
		}
		assert(node);
		assert(((communication*)info.handle)->id == info.id);
	}
#endif
	void disconnect(const connection_info& info) {
		//verify that the info has valid id
#ifndef NDEBUG
		verify_connection_info(info);
		assert((((communication*)info.handle)->state == uint32_t(comm_state::COMM_ACKED))
			|| (((communication*)info.handle)->state == uint32_t(comm_state::COMM_EST)));
#endif
		((communication*)info.handle)->state = uint32_t(comm_state::COMM_ENDWAIT);
		((communication*)info.handle)->post
			->emplace(message{EVENT_UNREGISTER,((communication*)info.handle)->type,
				((communication*)info.handle)->events, ((communication*)info.handle)->post});
	}
	void poll_incomming(const eventbox::message& msg) {
		//accept incomming connections
		rigtorp::SPSCQueue<message>* comming;
		while (applications.try_pop(comming)) {
			//process cur
			communication* com = connect_accept(*(comming->front()));
			if (com)
				static_cast<T*>(this)->register_incomming(msg, *com);
			//			comming->pop();
		}
	}
	bool process_comms(size_t max_event = SIZE_MAX) {
		bool rtn = true;
		comm_node* node = head;
		uint32_t i = 0;
		while (node) {
			for (size_t j = 0; j < COM_SEG && i < num_used; ++j, ++i) {
				if (uint32_t(node->comms[j].state) && node->comms[j].events->front()) {
					communication& com = node->comms[j];
					size_t k = 0;
					for (; k < max_event && uint32_t(node->comms[j].state); ++k) {
						message* msg = com.events->front();
						if (!msg) break;
						//process msg
						bool end = false;
						switch (process_single_event(*msg, com)) {
							case 0:
								break;
							case 1:
								end = true;
								com.events->pop();
								break;
							case 2:
								end = true;
								break;
						}
						if (end)
							break;
						com.events->pop();
					}
					if (k == max_event) {
						rtn = false;
						com.post->emplace(
							message{ EVENT_CANNOT_PROCESS_ALL, 0, com.events,com.post });
					}
				}
			}
			node = node->next;
		}
		return rtn;
	}
	bool process_events(size_t max_event = SIZE_MAX) {
		poll_incomming();
		//process events
		return process_comms(max_event);
	};
	void connect_cancel(const connection_info& con) {
#ifndef NDEBUG
		verify_connection_info(con);
		assert(((communication*)con.handle)->state == communication::COMM_PENDING);
#endif
		((communication*)con.handle)->post
			->emplace(message{ EVENT_REGISTER_CANCEL, 
			((communication*)con.handle)->type, ((communication*)con.handle)->events, ((communication*)con.handle)->post });
		recycle_comm_slot((communication*)con.handle);
	}
	static uint32_t get_state(const connection_info& info) {
		return ((communication*)info.handle)->state;
	}
	static uint32_t get_type(const connection_info& info) {
		return ((communication*)info.handle)->type;
	}
	bool try_post_event(const connection_info& info, message&& message) {
		assert(info.handle);
		communication& com = *((communication*)info.handle);
#ifndef NDEBUG
		assert(com.id == info.id);
#endif
		return com.post->try_emplace(message);
	}
	//may block
	void post_event(const connection_info& info, message&& message) {
		assert(info.handle);
		communication& com = *((communication*)info.handle);
#ifndef NDEBUG
		assert(com.id == info.id);
#endif
		com.post->emplace(message);
	}
	eventbox(const eventbox&) = delete;
	eventbox(eventbox&&) = delete;
	eventbox& operator=(const eventbox&) = delete;
	eventbox& operator=(eventbox&&) = delete;
	eventbox(size_t listen_size = 16) :applications(listen_size) {};
	bool resize_recycle() {
		bool cleaned = false;
		if (recycle_size) {
			if (num_recycled)
				recycled = (communication**)realloc(recycled, num_recycled * sizeof(communication*));
			else {
				free(recycled);
				cleaned = true;
			}
		}
		return cleaned;
	}
	bool collect_no_recycle() {
		if (queue_areas) {
			clear_queue_areas(queue_areas);
			queue_areas = nullptr;
		}
		if (head) {
			if (clean_comms(head))
				head = nullptr;
		}
		if (!head) {
			return true;
		}
		return false;
	}
	bool collect() {
		return collect_no_recycle() && resize_recycle();
	}
	//applications not dealt with
	~eventbox() {
		do {
			while (!collect_no_recycle()) {
				size_t node_index = num_used / COM_SEG;
				size_t index = num_used % COM_SEG;
				comm_node* node = head;
				for (size_t i = 0; i < node_index; ++i) {
					for (size_t j = 0; j < COM_SEG; ++j) {
						switch (node->comms[j].state) {
							case uint32_t(comm_state::COMM_NONE):
							case uint32_t(comm_state::COMM_ENDWAIT):
								break;
							case uint32_t(comm_state::COMM_PENDING):
								node->comms[j].post
									->emplace(message{ EVENT_REGISTER_CANCEL,node->comms[j].type,
										node->comms[j].events, node->comms[j].post });
								recycle_comm_slot(&(node->comms[j]));
								break;
							case uint32_t(comm_state::COMM_ACKED):
							case uint32_t(comm_state::COMM_EST):
								node->comms[j].state = uint32_t(comm_state::COMM_ENDWAIT);
								node->comms[j].post
									->emplace(message{ EVENT_UNREGISTER,node->comms[j].type,
										node->comms[j].events, node->comms[j].post });
								break;
							default:
								abort();
						}
					}
					assert(node->next);
					node = node->next;
				}
				for (size_t j = 0; j < index; ++j) {
					switch (node->comms[j].state) {
					case uint32_t(comm_state::COMM_NONE):
					case uint32_t(comm_state::COMM_ENDWAIT):
						break;
					case uint32_t(comm_state::COMM_PENDING):
						node->comms[j].post
							->emplace(message{ EVENT_REGISTER_CANCEL,node->comms[j].type,
								node->comms[j].events, node->comms[j].post });
						recycle_comm_slot(&(node->comms[j]));
						break;
					case uint32_t(comm_state::COMM_ACKED):
					case uint32_t(comm_state::COMM_EST):
						node->comms[j].state = uint32_t(comm_state::COMM_ENDWAIT);
						node->comms[j].post
							->emplace(message{ EVENT_UNREGISTER,node->comms[j].type,
								node->comms[j].events, node->comms[j].post });
						break;
					}
				}
				process_comms();
			}
		} while (!resize_recycle());
	}
};

//constexpr size_t com_seg = COM_SEG;
#undef COM_SEG


#include <iostream>
class test: public eventbox<test> {
public:
	int process_specific_event(const eventbox::message& msg, communication& com) {
		if (msg.type_global == EVENT_BASE_LAST) {
			++counter;
		}
//			std::cout << "Generic event recieved\n";
		post_event(peer, message{ EVENT_BASE_LAST,0,0,0 });
		return 0;
	};
	void register_incomming(const eventbox::message& msg, communication& comm) {
#ifndef NDEBUG
		peer = { &comm,comm.id };
#else
		peer = { comm };
#endif
		initialized = true;
	}
	test(size_t listen_size = 16): eventbox(listen_size) {};
	connection_info peer;
	bool initialized = false;
	size_t counter = 0;
};

constexpr size_t size = sizeof(rigtorp::SPSCQueue<nullptr_t>);


