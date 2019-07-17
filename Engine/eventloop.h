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
	struct message {
		uint64_t type;
		rigtorp::SPSCQueue<message>* callback;
		void* msg;
	};
	struct communication {
		enum {
			COMM_NONE,
			COMM_PENDING,				//applicant sent, no ack
			COMM_ACKED,					//acknowledgement sent, same as est
			COMM_EST,					//communication established
			COMM_ENDWAIT				//unregister message sent
		};
		uint64_t state;							//the current state of communication
//		eventbox* peer;							//the peer to send message to
		rigtorp::SPSCQueue<message>* events;	//the queue to recieve message from
		rigtorp::SPSCQueue<message>* post;		//the queue to send message to
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
	communication** port_recycled = nullptr;
	size_t num_recycled = 0;
	size_t recycle_size = 0;
	rigtorp::MPMCQueue<rigtorp::SPSCQueue<message>*> applications;
	//authoritative consumer actively register producer
	bool connect_accept(const message& msg) {
		assert(msg.type == EVENT_REGISTER);
		communication* comm = assign_port();
		if (!comm) {
			//deal with failed allocation
			msg.callback->emplace(message{ EVENT_REGISTER_FAIL , (rigtorp::SPSCQueue<message>*)this, msg.callback });
			return false;
		}
		*comm = { communication::COMM_ACKED, (rigtorp::SPSCQueue<message>*)msg.msg , msg.callback };
		comm->post->emplace(message{ EVENT_REGISTER_ACK, comm->events, comm->post });
		return true;
	}
	//authoritative consumer actively unregister producer
	//the producer is responsible for deleting the queue
	bool disconnect_accept(const message& msg, communication& com) {
		assert(msg.type == EVENT_UNREGISTER);
		assert(msg.callback == com.post);
		assert(msg.msg == com.events);
		if (num_recycled == recycle_size) {
			size_t new_size = size_t(num_recycled*1.618) + 1;
			communication** new_slots = 
				(communication**)realloc(port_recycled, new_size * sizeof(communication*));
			if (!new_slots) {
				com.post->emplace(message{ EVENT_UNREGISTER_FAIL,com.events,com.post });
				return false;
			}
			port_recycled = new_slots;
			recycle_size = new_size;
		}
		port_recycled[num_recycled] = &com;
		++num_recycled;
		com.state = communication::COMM_NONE;
		com.post->emplace(message{ EVENT_UNREGISTER_ACK,com.events,com.post });
	}
	void connect_establish(const message& msg, communication& com) {
		assert(msg.callback == com.events);
		assert(com.state == communication::COMM_PENDING);
		com.post = (rigtorp::SPSCQueue<message>*)msg.msg;
		com.state = communication::COMM_EST;
	}
	void disconnect_done(const message& msg, communication& com, uint32_t port) {
		assert(msg.type == EVENT_UNREGISTER_ACK);
		assert(msg.callback == com.post);
		assert(msg.msg == com.events);
		delete com.events;
		delete com.post;
		com.state = communication::COMM_NONE;
	}
	void connect_accept_fail(const message& msg, communication& com) {
		assert(msg.msg == com.events);
		assert(com.state == communication::COMM_PENDING);
		((eventbox*)msg.callback)->applications.emplace(com.events);
	}
	void disconnect_accept_fail(const message& msg, communication& com, uint32_t port) {
		assert(com.state == communication::COMM_ENDWAIT);
		assert((uint32_t)msg.msg == port);
		com.post->emplace(message{ EVENT_UNREGISTER,com.events,com.post });
	}
	communication* assign_port() {
		if (num_recycled) {
			assert(head);
			--num_recycled;
			return port_recycled[num_recycled];
		}
		if (num_used < num_comm) {
			assert(head);
			uint32_t port = num_used;
			uint32_t node_index = port / COM_SEG;
			if (!node_index) {
				return &(head->comms[port]);
			}
			uint32_t index = port % COM_SEG;
			comm_node* node = head;
			for (uint32_t i = 0; i < node_index; ++i) {
				assert(node->next);
				node = node->next;
			}
			return &(head->comms[index]);
		}
		comm_node* new_page = (comm_node*)mi_aligned_alloc(4096, 4096);
		if (!new_page) {
			//deal with failed allocation
			return nullptr;
		}
		new_page->next = nullptr;
		if (!head) {
			head = new_page;
			return &(new_page->comms[0]);
		}
		comm_node* node = head;
		while (node->next)
			node = node->next;
		node->next = new_page;
		num_comm += COM_SEG;
		++num_used;
		return &(new_page->comms[0]);
	}
public:
	virtual void process_single_event(const message& msg, communication& com, uint32_t port) = 0;
	enum {
		EVENT_REGISTER,
		EVENT_REGISTER_ACK,
		EVENT_REGISTER_FAIL,
		EVENT_UNREGISTER,
		EVENT_UNREGISTER_ACK,
		EVENT_UNREGISTER_FAIL,
		EVENT_CANNOT_PROCESS_ALL,
		EVENT_REGISTER_PRIORITY		//not implemented
	};
	//call this in the producer instance before anything else.
	communication* connect(eventbox* peer, size_t max_events = 100) {
		communication* comm = assign_port();
		if (!comm) return nullptr;
		comm->state = communication::COMM_PENDING;
		comm->events = new rigtorp::SPSCQueue<message>(max_events);
		comm->post = new rigtorp::SPSCQueue<message>(max_events);
		comm->post->emplace(message{ EVENT_REGISTER, comm->events, comm->post });
		//current implementation cannot handle failed registration
		peer->applications.emplace(comm->post);
	}
	void apply_unregister(eventbox* producer, uint32_t port) {

	}
	//the port number is the port number pn the reciever side
	//maybe forcing a inline from calling places can 
	//reduce overhead (code cache miss)?
	void send_msg(uint32_t port, message msg) {
		assert(port < num_comm);
		if (port < COM_SEG)
			head->comms[port].events->emplace(msg);
		else {
			uint32_t node_index = port / COM_SEG;
			uint32_t index = port % COM_SEG;
			comm_node* node = head;
			for (uint32_t i = 0; i < node_index; ++i)
				node = node->next;
			node->comms[index].events->emplace(msg);
		}
	};
	bool process_events(size_t max_event = SIZE_MAX) {
		//accept incomming connections
		rigtorp::SPSCQueue<message>* comming;
		while (applications.try_pop(comming)) {
			//process cur
			connect_accept(*(comming->front()));
			comming->pop();
		}
		//process events
		bool rtn = true;
		comm_node* node = head;
		uint32_t i = 0;
		while (node) {
			for (size_t j = 0; j < COM_SEG && i < num_used; ++j, ++i) {
				if (node->comms[j].state) {
					communication& com = node->comms[j];
					size_t k = 0;
					for (; k < max_event; ++k) {
						message* msg = com.events->front();
						if (!msg) break;
						//process msg
						static_cast<T*>(this)->process_single_event(*msg, com, i);
					}
					if (k == max_event) { 
						rtn = false;
						com.peer->send_msg(com.port,
							message{ EVENT_CANNOT_PROCESS_ALL,this,(void*)(uint64_t(com.port) << 32 | i) });
					}
				}
			}
			node = node->next;
		}
		return rtn;
	};

	eventbox(const eventbox&) = delete;
	eventbox(eventbox&&) = delete;
	eventbox(size_t listen_size = 16) :applications(listen_size) {};
};

//constexpr size_t com_seg = COM_SEG;
#undef COM_SEG

class test :public eventbox<test> {
	void process_single_event(const eventbox::message& msg, communication& com, uint32_t port) {};
public:
	test(size_t listen_size = 16): eventbox(listen_size) {};
};



