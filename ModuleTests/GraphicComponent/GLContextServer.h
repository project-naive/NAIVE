#pragma once

#include <Core/component.h>
#include "GLContext.h"

constexpr size_t size = sizeof(Engine::Graphics::GLContext);

class GLContextServer : component<GLContextServer> {
public:
	//the specific field of this component
	enum :uint16_t {
		EVENT_CONTEXT_NEW,
		EVENT_CONTEXT_NEW_ACK,
		EVENT_CONTEXT_NEW_FAIL,
		EVENT_CONTEXT_DELETE,
		EVENT_CONTEXT_DELETE_ACK,
		EVENT_CONTEXT_DELETE_FAIL,
		EVENT_CONTEXT_TRANSFER,
		EVENT_CONTEXT_TRANSFER_ACK,
		EVENT_CONTEXT_TRANSFER_FAIL,
		//used to reclaim an offloaded context
		EVENT_CONTEXT_TRANSFER_CANCEL,
		EVENT_CONTEXT_SHOULD_END
	};
	//the type of connections available for this component
	enum :uint16_t {
		TYPE_CONTEXT_SERVER,
		TYPE_CONTEXT_CLIENT,
	};
private:
	GLContextServer(GLContextServer* mainthreads, Engine::Graphics::WindowInfo* info = nullptr):
		component<GLContextManager>(16) {

	};
	~GLContextServer() {
		
	};
public:
	struct ContextGenInfo {
		Engine::Graphics::WindowInfo info;
		Engine::Graphics::GLContext* shared;
	};
protected:
	friend Engine::Graphics::GLContext;
private:
	Engine::Graphics::GLContext** Contexts = nullptr;
	size_t context_count = 0;
	size_t context_size = 0;
	size_t* Recycled_context = nullptr;
	size_t Context_recycled = 0;
	size_t Context_recycle_size = 0;
	void GenContext(const message& msg, communication& comm) {
		assert(msg.type_global == EVENT_CONTEXT);
		assert(msg.type_specific == EVENT_CONTEXT_NEW);
		assert(msg.callback == comm.post);
		assert(comm.type == TYPE_CONTEXT_CLIENT);
		assert(comm.state == uint16_t(comm_state::COMM_ACKED));
		assert(msg.msg);
		ContextGenInfo& info = *((ContextGenInfo*)msg.msg);
		if (Context_recycled) {
			size_t slot = Recycled_context[Context_recycled - 1];
			new (Contexts[slot])Engine::Graphics::GLContext(&info.info, info.shared);
			--Context_recycled;
		}
		if (context_count == context_size) {
			size_t new_size = size_t(context_size*1.618) + 1;
			Engine::Graphics::GLContext** new_cache
				= (Engine::Graphics::GLContext**)realloc(Contexts, new_size * sizeof(Engine::Graphics::GLContext*));
			if (!new_cache) {
				msg.callback->emplace(message{ EVENT_CONTEXT,EVENT_CONTEXT_NEW_FAIL,comm.events,msg.msg });
			}
			Contexts = new_cache;
			context_size = new_size;
		}
		Contexts[context_count] = new Engine::Graphics::GLContext(&info.info, info.shared);
		++context_count;
		msg.callback->emplace(message{ EVENT_CONTEXT,EVENT_CONTEXT_NEW_ACK,comm.events,msg.msg });
	};
	void DelContext(const message& msg, communication& comm) {
		assert(msg.type_global == EVENT_CONTEXT);
		assert(msg.type_specific == EVENT_CONTEXT_DELETE);
		assert(msg.callback == comm.post);
		assert(comm.type == TYPE_CONTEXT_CLIENT);
		assert(comm.state == uint16_t(comm_state::COMM_ACKED));
		assert(msg.msg);
		Engine::Graphics::GLContext* context = (Engine::Graphics::GLContext*)msg.msg;
		if (Context_recycled == Context_recycle_size) {
			size_t new_size = size_t(Context_recycle_size*1.618) + 1;
			size_t* new_cache
				= (size_t*)realloc(Recycled_context, new_size * sizeof(size_t));
			if (!new_cache) {
				msg.callback->emplace(message{ EVENT_CONTEXT,EVENT_CONTEXT_DELETE_FAIL,comm.events,msg.msg });
			}
			Recycled_context = new_cache;
		}
		context->~GLContext();
		size_t i = 0;
		for (; i < context_count; ++i) {
			if (Contexts[i] == context) break;
		}
		assert(i < context_count);
		Recycled_context[Context_recycled] = i;
		++Context_recycled;
		msg.callback->emplace(message{ EVENT_CONTEXT,EVENT_CONTEXT_DELETE_ACK,comm.events,msg.msg });
	};
public:
	static int ContextManagerConnection(GLContextServer* __this, const message& msg, communication& com) {
		assert(msg.type_global == EVENT_CONTEXT);
		switch (msg.type_specific) {
			case EVENT_CONTEXT_NEW:
				__this->GenContext(msg, com);
				break;
			case EVENT_CONTEXT_DELETE:
				__this->DelContext(msg, com);
				break;
		}
	};
	//change to add an array to hold things...
	void register_incomming(const message& msg, communication& comm) {
		comm.callback = ContextManagerConnection;
		++num_managers;
	};
	void process_noconnect(const message& msg) {

	};
	size_t num_managers = 0;
public:
	//This is a singleton manager with thread-local storage
	static GLContextServer& GetInst(GLContextServer* main_thread, Engine::Graphics::WindowInfo* info = nullptr) {
		//assert: this thread is main thread
		static GLContextServer server(main_thread, info);
		return server;
	}
};