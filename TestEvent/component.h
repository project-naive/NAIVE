#pragma once

#include <cstdint>
#include <rigtorp/SPSCQueue.h>
#include <rigtorp/MPMCQueue.h>
#include <MI/mimalloc.h>

struct connection_info;
struct message {
	uint32_t type_global;
	uint32_t type_specific;
	rigtorp::SPSCQueue<message>* callback;
	void* msg;
};

//This class implements a method of lock-free inter-module
//communication. The handle for a communication consists of
//a pointer to target and the target's port number. Since
//most of in-game modules have a relatively long lifetime
//compared to a loop round, the data structure uses a linked
//list of many connections as blocks. If connections were to be
//created and destroyed frequently, a doubly linked list with
//recycling may be better
template<typename T>
class component {
public:
	struct communication;
	component(const component&) = delete;
	component(component&&) = delete;
	component& operator=(const component&) = delete;
	component& operator=(component&&) = delete;
	component(size_t listen_size = 16) :MPSC_data(listen_size) {};
	//MPSC_DATA not dealt with
	~component();
	void* operator new(size_t size) {
		return mi_aligned_alloc(128, size);
	}
	void operator delete(void* ptr) {
		mi_free_aligned(ptr, 128);
	}
	enum class comm_state: uint16_t {
		COMM_NONE,
		COMM_PENDING,				//applicant sent, no ack
		COMM_ACKED,					//acknowledgement sent, same as est
		COMM_EST,					//communication established
		COMM_ENDWAIT				//unregister message sent
	};
	enum :uint16_t {
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
	rigtorp::MPMCQueue<message> MPSC_data;
	//call this in the producer instance before anything else.
	template<typename P>
	inline connection_info connect(
			component<P>* peer, int(*callback)(T*, const message&, communication&),
			uint16_t self_type, uint16_t peer_type,
			uint16_t self_events=100, uint16_t peer_events=100);
	void disconnect(const connection_info& info);
	void connect_cancel(const connection_info& con);
	static uint16_t get_state(const connection_info& info);
	static uint16_t get_type(const connection_info& info);
	void poll_incomming();
	bool process_comms(size_t max_event = SIZE_MAX);
	bool process_events(size_t max_event = SIZE_MAX);
	//may block
	void post_event(connection_info& info, const message& message);
	bool try_post_event(const connection_info& info, message&& message);
	//memory reclaims
	bool resize_recycle();
	bool collect_no_recycle();
	bool collect();
protected:
	communication* connect_accept(const message& msg);
	void connect_establish(const message& msg, communication& com);
	void connect_accept_cancel(const message& msg, communication& com);
	void connect_accept_fail(const message& msg, communication& com);
	bool disconnect_accept(const message& msg, communication& com);
	void disconnect_done(const message& msg, communication& com);
	void disconnect_accept_fail(const message& msg, communication& com);
	int process_single_event(const message& msg, communication& com);
	uint32_t id_max = 1;	//id = 0 means invalid
#ifndef NDEBUG
	void verify_connection_info(const connection_info& info);
#endif
private:
#define COM_SEG ((4096 - 8) / sizeof(communication))
	struct alignas(4096) comm_node {
		alignas(4096 - COM_SEG * sizeof(communication)) comm_node* next;
		communication comms[COM_SEG];
	};
	//do not need to know about padding
	struct recycled_block {
		recycled_block* next;
	};
	communication* assign_port();
	bool recycle_comm_slot(communication* com);
	rigtorp::SPSCQueue<message>* assign_queue_block();
	void recycle_queue_block(rigtorp::SPSCQueue<message>* p);
	bool clean_comms(comm_node* node, size_t recurse = 0);
	//This function may be improved by finding all in one round
	void remove_recycle(communication* start, communication* end);
	void clear_queue_areas(recycled_block* node = head);
	comm_node* head = nullptr;
	size_t num_comm = 0;
	size_t num_used = 0;
	communication** recycled = nullptr;
	size_t num_recycled = 0;
	size_t recycle_size = 0;
	recycled_block* queue_areas = nullptr;
};
#include "component.ipp"
#undef COM_SEG
