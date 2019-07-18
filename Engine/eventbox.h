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
		comm_state state;							//the current state of communication
		uint32_t type;
		rigtorp::SPSCQueue<message>* events;	//the queue to recieve message from
		rigtorp::SPSCQueue<message>* post;		//the queue to send message to
		int64_t id;
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
	rigtorp::MPMCQueue<rigtorp::SPSCQueue<message>*> applications;
	bool recycle_comm_slot(communication* com) {
#ifndef NDEBUG
		assert(com);
#endif
		com->id = -1;
		com->state = comm_state::COMM_NONE;
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
		return true;
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
		*comm = { comm_state::COMM_ACKED, msg.type_specific,(rigtorp::SPSCQueue<message>*)msg.msg , msg.callback };
		comm->post->emplace(message{ EVENT_REGISTER_ACK, msg.type_specific, comm->events, comm->post });
		return comm;
	}
	//authoritative consumer actively unregister producer
	//the producer is responsible for deleting the queue
	bool disconnect_accept(const message& msg, communication& com) {
		assert(msg.type_global == EVENT_UNREGISTER);
		assert(msg.callback == com.post);
		assert(msg.msg == com.events);
		com.state = comm_state::COMM_NONE;
		com.post->emplace(message{
			EVENT_UNREGISTER_ACK, msg.type_specific,(rigtorp::SPSCQueue<message>*)this, com.post });
		return recycle_comm_slot(&com);
	}
	void connect_establish(const message& msg, communication& com) {
		assert(msg.callback == com.post);
		assert(msg.msg == com.events);
		assert(com.state == comm_state::COMM_PENDING);
		assert(msg.type_global == EVENT_REGISTER_ACK);
		com.state = comm_state::COMM_EST;
	}
	void connect_accept_cancel(const message& msg, communication& com) {
		assert(com.state == comm_state::COMM_ACKED);
		assert(msg.type_global == EVENT_REGISTER_CANCEL);
		assert(msg.msg == com.events);
		assert(msg.callback == com.post);
		delete com.post;
		delete com.events;
		recycle_comm_slot(&com);
	}
	void disconnect_done(const message& msg, communication& com) {
		assert(msg.type_global == EVENT_UNREGISTER_ACK);
		assert(msg.msg == com.events);
		delete com.events;
		delete com.post;
		com.state = comm_state::COMM_NONE;
		recycle_comm_slot(&com);
	}
	void connect_accept_fail(const message& msg, communication& com) {
		assert(msg.msg == com.events);
		assert(com.state == comm_state::COMM_PENDING);
		((eventbox*)msg.callback)->applications.emplace(com.events);
	}
	communication* assign_port() {
		if (num_recycled) {
			assert(head);
			--num_recycled;
			communication* ptr = recycled[num_recycled];
			ptr->state = comm_state::COMM_NONE;
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
			ptr->state = comm_state::COMM_NONE;
			ptr->id = id_max;
			++id_max;
			return ptr;
		}
		comm_node* new_page = (comm_node*)mi_aligned_alloc(4096, 4096);
		if (!new_page) {
			//deal with failed allocation
			return false;
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
		ptr->state = comm_state::COMM_NONE;
		ptr->id = id_max;
		++id_max;
		return ptr;
	}
//	virtual void process_single_event(const message& msg, communication& com) = 0;
	int64_t id_max = 0;	//id = -1 means invalid
public:
	enum : uint64_t{
		EVENT_REGISTER,
		EVENT_REGISTER_CANCEL,
		EVENT_REGISTER_ACK,
		EVENT_REGISTER_FAIL,
		EVENT_UNREGISTER,
		EVENT_UNREGISTER_CANCEL,	//impossible
		EVENT_UNREGISTER_ACK,
		EVENT_UNREGISTER_FAIL,		//not implemented
		EVENT_CANNOT_PROCESS_ALL,
		EVENT_REGISTER_PRIORITY		//not implemented
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
		int64_t id;		//for verification
#endif
	};
	//call this in the producer instance before anything else.
	connection_info connect(eventbox* peer,
		uint32_t self_type, uint32_t peer_type, 
		uint32_t self_events = 100, uint32_t peer_events = 100) {
		communication* comm = assign_port();
#ifndef NDEBUG
		if (!comm) return { nullptr, -1 };
#else
		if (!comm) return { nullptr };
#endif
		comm->state = comm_state::COMM_PENDING;
		comm->events = new rigtorp::SPSCQueue<message>(self_events);
		comm->post = new rigtorp::SPSCQueue<message>(peer_events);
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
			if (((communication*)info.handle) - node->comms < COM_SEG)
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
		assert((((communication*)info.handle)->state == comm_state::COMM_ACKED)
			|| (((communication*)info.handle)->state == comm_state::COMM_EST));
#endif
		((communication*)info.handle)->state = comm_state::COMM_ENDWAIT;
		((communication*)info.handle)->post
			->emplace(message{EVENT_UNREGISTER,((communication*)info.handle)->type,
				((communication*)info.handle)->events, ((communication*)info.handle)->post});
	}
	bool process_events(size_t max_event = SIZE_MAX) {
		//accept incomming connections
		rigtorp::SPSCQueue<message>* comming;
		while (applications.try_pop(comming)) {
			//process cur
			communication* com = connect_accept(*(comming->front()));
			if (com)
				static_cast<T*>(this)->register_incomming(com);
//			comming->pop();
		}
		//process events
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
						switch (static_cast<T*>(this)->process_single_event(*msg, com)) {
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
						if(end)
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
	static comm_state get_state(const connection_info& info) {
		return ((communication*)info.handle)->state;
	}
	static uint32_t get_type(const connection_info& info) {
		return ((communication*)info.handle)->type;
	}
	eventbox(const eventbox&) = delete;
	eventbox(eventbox&&) = delete;
	eventbox& operator=(const eventbox&) = delete;
	eventbox& operator=(eventbox&&) = delete;
	eventbox(size_t listen_size = 16) :applications(listen_size) {};
};

//constexpr size_t com_seg = COM_SEG;
#undef COM_SEG


#include <iostream>
class test: public eventbox<test> {
public:
	int process_single_event(const eventbox::message& msg, communication& com) {
		switch (msg.type_global) {
			case EVENT_REGISTER:
				std::cout << "EVENT_REGISTER GOT.\n";
				return 0;
			case EVENT_REGISTER_CANCEL:
				std::cout << "EVENT_REGISTER_CANCEL GOT.\n";
				connect_accept_cancel(msg, com);
				return 2;
			case EVENT_REGISTER_ACK:
				std::cout << "EVENT_REGISTER_ACK GOT.\n";
				connect_establish(msg, com);
				return 0;
			case EVENT_REGISTER_FAIL:
				std::cout << "EVENT_REGISTER_FAIL GOT.\n";
				connect_accept_fail(msg, com);
				return 1;
			case EVENT_UNREGISTER:
				std::cout << "EVENT_UNREGISTER GOT.\n";
				disconnect_accept(msg, com);
				return 2;
			case EVENT_UNREGISTER_CANCEL:
				std::cout << "EVENT_UNREGISTER_CANCEL GOT.\n";
				abort();
				break;
			case EVENT_UNREGISTER_ACK:
				std::cout << "EVENT_UNREGISTER_ACK GOT.\n";
				disconnect_done(msg, com);
				return 2;
			case EVENT_UNREGISTER_FAIL:
				std::cout << "EVENT_UNREGISTER_FAIL GOT.\n";
				abort();
				break;
			default:
				return 0;
		}
		return 1;
	};
	void register_incomming(communication* comm) {
#ifndef NDEBUG
		peer = { comm,comm->id };
#else
		peer = { comm };
#endif
		initialized = true;
	}
	test(size_t listen_size = 16): eventbox(listen_size) {};
	connection_info peer;
	bool initialized = false;
};



