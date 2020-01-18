struct connection_info {
	void* handle;	//handle of a communication, internally a communication*
#ifndef NDEBUG
	uint64_t id;		//for verification
#endif
};

template<typename T>
struct component<T>::communication {
	uint16_t state;							//the current state of communication
	uint16_t type;
	uint32_t id;
	rigtorp::SPSCQueue<message>* events;	//the queue to recieve message from
	rigtorp::SPSCQueue<message>* post;		//the queue to send message to
	int(*callback)(T* ,const message&, communication&);
};

//MPSC_data not dealt with
template<typename T>
inline component<T>::~component() {
	do {
		while (!collect_no_recycle()) {
			size_t node_index = num_used / COM_SEG;
			size_t index = num_used % COM_SEG;
			comm_node* node = head;
			for (size_t i = 0; i < node_index; ++i) {
				for (size_t j = 0; j < COM_SEG; ++j) {
					switch (node->comms[j].state) {
					case uint16_t(comm_state::COMM_NONE):
					case uint16_t(comm_state::COMM_ENDWAIT):
						break;
					case uint16_t(comm_state::COMM_PENDING):
						node->comms[j].post
							->emplace(message{ EVENT_REGISTER_CANCEL,node->comms[j].type,
								node->comms[j].events, node->comms[j].post });
						recycle_comm_slot(&(node->comms[j]));
						break;
					case uint16_t(comm_state::COMM_ACKED):
					case uint16_t(comm_state::COMM_EST):
						node->comms[j].state = uint16_t(comm_state::COMM_ENDWAIT);
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
				case uint16_t(comm_state::COMM_NONE):
				case uint16_t(comm_state::COMM_ENDWAIT):
					break;
				case uint16_t(comm_state::COMM_PENDING):
					node->comms[j].post
						->emplace(message{ EVENT_REGISTER_CANCEL,node->comms[j].type,
							node->comms[j].events, node->comms[j].post });
					recycle_comm_slot(&(node->comms[j]));
					break;
				case uint16_t(comm_state::COMM_ACKED):
				case uint16_t(comm_state::COMM_EST):
					node->comms[j].state = uint16_t(comm_state::COMM_ENDWAIT);
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

//call this in the producer instance before anything else.
template<typename T>
template<typename P>
inline connection_info
component<T>::connect(
	component<P>* peer, int(*callback)(T* ,const message&, communication&),
	uint16_t self_type, uint16_t peer_type,
	uint16_t self_events, uint16_t peer_events) {
	communication* comm = assign_port();
#ifndef NDEBUG
	if (!comm) return { nullptr, 0 };
#else
	if (!comm) return { nullptr };
#endif
	comm->state = uint16_t(comm_state::COMM_PENDING);
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
	comm->callback = callback;
	comm->post->emplace(message{ EVENT_REGISTER, peer_type,comm->events, comm->post });
	//current implementation cannot handle failed registration
	peer->MPSC_data.emplace(message{ EVENT_REGISTER, peer_type,comm->events, comm->post });
#ifndef NDEBUG
	return { comm, comm->id };
#else
	return { comm };
#endif
}

template<typename T>
inline void component<T>::disconnect(const connection_info & info) {
	//verify that the info has valid id
#ifndef NDEBUG
	verify_connection_info(info);
	assert((((communication*)info.handle)->state == uint16_t(comm_state::COMM_ACKED))
		|| (((communication*)info.handle)->state == uint16_t(comm_state::COMM_EST)));
#endif
	((communication*)info.handle)->state = uint16_t(comm_state::COMM_ENDWAIT);
	((communication*)info.handle)->post
		->emplace(message{ EVENT_UNREGISTER,((communication*)info.handle)->type,
		((communication*)info.handle)->events, ((communication*)info.handle)->post });
}

template<typename T>
inline void component<T>::connect_cancel(const connection_info & con) {
#ifndef NDEBUG
	verify_connection_info(con);
	assert(((communication*)con.handle)->state == uint16_t(comm_state::COMM_PENDING));
#endif
	((communication*)con.handle)->post
		->emplace(message{ EVENT_REGISTER_CANCEL,
		((communication*)con.handle)->type, ((communication*)con.handle)->events, ((communication*)con.handle)->post });
	recycle_comm_slot((communication*)con.handle);
}

template<typename T>
inline uint16_t component<T>::get_state(const connection_info & info) {
	return ((communication*)info.handle)->state;
}

template<typename T>
inline uint16_t component<T>::get_type(const connection_info & info) {
	return ((communication*)info.handle)->type;
}

template<typename T>
inline void component<T>::poll_incomming() {
	//accept incomming connections
	message comming;
	while (MPSC_data.try_pop(comming)) {
		//process cur
		communication* com = nullptr;
		switch (comming.type_global) {
		case EVENT_REGISTER:
			com = connect_accept(comming);
			break;
		default:
			static_cast<T*>(this)->process_noconnect(comming);
		}


		//			comming->pop();
	}
}

template<typename T>
inline bool component<T>::process_comms(size_t max_event) {
	bool rtn = true;
	comm_node* node = head;
	uint32_t i = 0;
	while (node) {
		for (size_t j = 0; j < COM_SEG && i < num_used; ++j, ++i) {
			if (uint16_t(node->comms[j].state) && node->comms[j].events->front()) {
				communication& com = node->comms[j];
				size_t k = 0;
				for (; k < max_event && uint16_t(node->comms[j].state); ++k) {
					message* msg = com.events->front();
					if (!msg) break;
					//process msg
					bool end = false;
					int code;
					switch (msg->type_global) {
						case EVENT_REGISTER:
							code = 0;
							break;
						case EVENT_REGISTER_CANCEL:
							connect_accept_cancel(*msg, com);
							code = 2;
							break;
						case EVENT_REGISTER_ACK:
							connect_establish(*msg, com);
							code = 0;
							break;
						case EVENT_REGISTER_FAIL:
							connect_accept_fail(*msg, com);
							code = 1;
							break;
						case EVENT_UNREGISTER:
							disconnect_accept(*msg, com);
							code = 2;
							break;
						case EVENT_UNREGISTER_CANCEL:
							abort();
							code = 0;
							break;
						case EVENT_UNREGISTER_ACK:
							disconnect_done(*msg, com);
							code = 2;
							break;
						case EVENT_UNREGISTER_FAIL:
							disconnect_accept_fail(*msg, com);
							code = 1;
							break;
						default:
							code = com.callback(static_cast<T*>(this), *msg, com);
					}
					switch (code) {
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

template<typename T>
inline bool component<T>::process_events(size_t max_event) {
	poll_incomming();
	//process events
	return process_comms(max_event);
}

//may block
template<typename T>
inline void component<T>::post_event(connection_info & info, const message & message) {
	assert(info.handle);
	communication& com = *((communication*)info.handle);
#ifndef NDEBUG
	assert(com.id == info.id);
#endif
	com.post->emplace(message);
}

template<typename T>
inline bool component<T>::try_post_event(const connection_info & info, message && message) {
	assert(info.handle);
	communication& com = *((communication*)info.handle);
#ifndef NDEBUG
	assert(com.id == info.id);
#endif
	return com.post->try_emplace(message);
}

template<typename T>
inline bool component<T>::resize_recycle() {
	bool cleaned = false;
	if (recycle_size) {
		if (num_recycled)
			recycled = (communication**)realloc(recycled, num_recycled * sizeof(communication*));
		else {
			free(recycled);
			recycle_size = 0;
			cleaned = true;
		}
	}
	return cleaned;
}

template<typename T>
inline bool component<T>::collect_no_recycle() {
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

template<typename T>
inline bool component<T>::collect() {
	return collect_no_recycle() && resize_recycle();
}

template<typename T>
inline typename component<T>::communication* component<T>::connect_accept(const message & msg) {
	assert(msg.type_global == EVENT_REGISTER);
	communication* comm = assign_port();
	if (!comm) {
		//deal with failed allocation
		msg.callback->emplace(message{ EVENT_REGISTER_FAIL, msg.type_specific,
			(rigtorp::SPSCQueue<message>*)(&(this->MPSC_data)), msg.callback });
		return nullptr;
	}
	comm->state = uint16_t(comm_state::COMM_ACKED);
	comm->type = msg.type_specific;
	comm->events = (rigtorp::SPSCQueue<message>*)msg.msg;
	comm->post = msg.callback;
	static_cast<T*>(this)->register_incomming(msg, *comm);
	comm->post->emplace(message{ EVENT_REGISTER_ACK, msg.type_specific, comm->events, comm->post });
	return comm;
}

template<typename T>
inline void component<T>::connect_establish(const message & msg, communication & com) {
	assert(msg.callback == com.post);
	assert(msg.msg == com.events);
	assert(com.state == uint16_t(comm_state::COMM_PENDING));
	assert(msg.type_global == EVENT_REGISTER_ACK);
	com.state = uint16_t(comm_state::COMM_EST);
}

template<typename T>
inline void component<T>::connect_accept_cancel(const message & msg, communication & com) {
	assert(com.state == uint16_t(comm_state::COMM_ACKED));
	assert(msg.type_global == EVENT_REGISTER_CANCEL);
	assert(msg.msg == com.events);
	assert(msg.callback == com.post);
	(*com.post).~SPSCQueue<message>();
	recycle_queue_block(com.post);
	(*com.events).~SPSCQueue<message>();
	recycle_queue_block(com.events);
	recycle_comm_slot(&com);
}

template<typename T>
inline void component<T>::connect_accept_fail(const message & msg, communication & com) {
	assert(msg.msg == com.events);
	assert(com.state == uint16_t(comm_state::COMM_PENDING));
	((rigtorp::MPMCQueue<message>*)msg.callback)->emplace(message{ EVENT_REGISTER,msg.type_specific,com.events,com.post });
}

template<typename T>
inline bool component<T>::disconnect_accept(const message & msg, communication & com) {
	assert(msg.type_global == EVENT_UNREGISTER);
	assert(msg.callback == com.post);
	assert(msg.msg == com.events);
	if (!recycle_comm_slot(&com)) {
		com.post->emplace(message{
			EVENT_UNREGISTER_FAIL, msg.type_specific,com.events, com.post });
		return false;
	}
	com.state = uint16_t(comm_state::COMM_NONE);
	com.post->emplace(message{
		EVENT_UNREGISTER_ACK, msg.type_specific,(rigtorp::SPSCQueue<message>*)this, com.post });
	return true;
}

template<typename T>
inline void component<T>::disconnect_done(const message & msg, communication & com) {
	assert(msg.type_global == EVENT_UNREGISTER_ACK);
	assert(msg.msg == com.events);
	com.post->~SPSCQueue();
	recycle_queue_block(com.post);
	com.events->~SPSCQueue();
	recycle_queue_block(com.events);
	com.state = uint16_t(comm_state::COMM_NONE);
	recycle_comm_slot(&com);
}

template<typename T>
inline void component<T>::disconnect_accept_fail(const message & msg, communication & com) {
	assert(msg.callback == com.post);
	assert(msg.msg == com.events);
	assert(msg.type_global == EVENT_UNREGISTER_FAIL);
	com.post->emplace(message{ EVENT_UNREGISTER, msg.type_specific, com.events,com.post });
}

#ifndef NDEBUG
template<typename T>
inline void component<T>::verify_connection_info(const connection_info & info) {
	assert(head);
	comm_node* node = head;
	while (node) {
		if ((((communication*)info.handle) - node->comms) >= 0 &&
			(((communication*)info.handle) - node->comms) < COM_SEG)
			break;
		node = node->next;
	}
	assert(node);
	assert(((communication*)info.handle)->id == info.id);
}
#endif

template<typename T>
inline typename component<T>::communication* component<T>::assign_port() {
	if (num_recycled) {
		assert(head);
		--num_recycled;
		communication* ptr = recycled[num_recycled];
		ptr->state = uint16_t(comm_state::COMM_NONE);
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
		ptr->state = uint16_t(comm_state::COMM_NONE);
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
	ptr->state = uint16_t(comm_state::COMM_NONE);
	ptr->id = id_max;
	++id_max;
	return ptr;
}

template<typename T>
inline bool component<T>::recycle_comm_slot(component<T>::communication * com) {
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
	com->state = uint16_t(comm_state::COMM_NONE);
	return true;
}

template<typename T>
inline rigtorp::SPSCQueue<message>* component<T>::assign_queue_block() {
	if (queue_areas) {
		rigtorp::SPSCQueue<message>* ptr = (rigtorp::SPSCQueue<message>*)queue_areas;
		queue_areas = queue_areas->next;
		return ptr;
	}
	return (rigtorp::SPSCQueue<message>*)malloc(sizeof(rigtorp::SPSCQueue<message>));
}

template<typename T>
inline void component<T>::recycle_queue_block(rigtorp::SPSCQueue<message>* p) {
	if (!queue_areas) {
		queue_areas = (recycled_block*)p;
		queue_areas->next = nullptr;
		return;
	}
	((recycled_block*)p)->next = queue_areas;
	queue_areas = (recycled_block*)p;
}

template<typename T>
inline bool component<T>::clean_comms(comm_node * node, size_t recurse) {
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
		remove_recycle(node->comms, node->comms + COM_SEG);
		num_comm -= COM_SEG;
		mi_free_aligned(node, 4096);
		return okay;
	}
	if (next_okay) {
		remove_recycle(start + 1, node->comms + COM_SEG);
		node->next = nullptr;
	}
	return okay;
}

template<typename T>
inline void component<T>::remove_recycle(communication * start, communication * end) {
	assert(start);
	assert(end);
	size_t num_found = 0;
	for (size_t i = 0; i < num_recycled - num_found;) {
		if (recycled[i + num_found] >= start && recycled[i + num_found] < end) {
			++num_found;
		}
		else {
			recycled[i] = recycled[i + num_found];
			++i;
		}
	}
	num_recycled -= num_found;
}

template<typename T>
inline void component<T>::clear_queue_areas(recycled_block* node) {
	if (node->next) {
		clear_queue_areas(node->next);
	}
	free(node);
}
