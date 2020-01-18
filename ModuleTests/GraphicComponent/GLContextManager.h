#pragma once

#include <Core/component.h>
#include "GLContext.h"

constexpr size_t size = sizeof(Engine::Graphics::GLContext);

class GLContextManager : component<GLContextManager> {
	//the global field of this component
	enum :uint16_t {
		EVENT_CONTEXT_NEW = EVENT_BASE_LAST,
		EVENT_CONTEXT_DELETE,
		EVENT_CONTEXT_TRANSFER,
		EVENT_CONTEXT_TRANSFER_ACK,
		//used to reclaim an offloaded context
		EVENT_CONTEXT_TRANSFER_CANCEL,
		EVENT_STATUS_RETRIEVE,
	};
	//the type of connections available for this component
	enum :uint16_t {
		TYPE_CONTEXT_SERVER,
		TYPE_CONTEXT_CLIENT,
	};
	public:
	GLContextManager(GLContextManager* mainthreads, Engine::Graphics::WindowInfo* info=nullptr);
	~GLContextManager();
protected:
	friend Engine::Graphics::GLContext;
	const size_t id;
private:
	static std::atomic<size_t> volatile id_max;
	static std::atomic<size_t> volatile instance_count;
	Engine::Graphics::GLContext** Contexts = nullptr;
	size_t context_count = 0;
	size_t context_size = 0;
	size_t current = -1;
	Engine::Graphics::WindowInfo::Window_Basic_Info display;
	glm::mat4 projection;
	struct	recycled_GLContext {
		size_t index;
	};
	recycled_GLContext** Recycled_context = nullptr;
	size_t Context_recycled = 0;
	size_t Context_recycle_size = 0;
	GLContextManager* const main_threads;
	connection_info main_thread_comm;
public:
	size_t GenContext(Engine::Graphics::WindowInfo& info, size_t shared_ID = -1);
	bool SetContext(size_t ID);
	bool DelContext(size_t ID);
	bool reserve(size_t size);
	bool EnableGLDebug(GLDEBUGPROC debugCallback);
	static int ContextManagerConnection(GLContextManager* __this, const message& msg, communication& com) {

	};
	static int process_specific_event(GLContextManager* __this, const message& msg, communication& com) {

	};
	void register_incomming(const message& msg, communication& comm) {

	}
	void process_noconnect(const message& msg) {
	
	};
public:
	//This is a singleton manager with thread-local storage
	static GLContextManager& GetInst(GLContextManager* main_thread, Engine::Graphics::WindowInfo* info=nullptr) {
		assert(main_thread->id == 0);
		static thread_local GLContextManager mgr(main_thread,info);
		return mgr;
	}
};


