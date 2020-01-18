#include "GLContextManager.h"

std::atomic<size_t> volatile GLContextManager::instance_count = 0;
std::atomic<size_t> volatile GLContextManager::id_max = 0;

#include <iostream>
#include <MI/mimalloc.h>

void glfwErrorCallback(int err_code, const char* info) {
	std::cerr << "Error " << err_code << ":\n" << info << '\n' << std::endl;
}

inline GLContextManager::GLContextManager(
	GLContextManager* mainthreads,
	Engine::Graphics::WindowInfo* info) : 
	component<GLContextManager>(16),
	id(id_max.fetch_add(1)),
	main_threads(mainthreads){
	//This is the main context on the main thread
	if (!id) {
		//create callback context
		glfwSetErrorCallback(glfwErrorCallback);
		std::clog << "GLFW Version string:\n"
			<< glfwGetVersionString() << '\n' << std::endl;
		if (!glfwInit()) {
			std::cerr << "Error initializing glfw!\n" << std::endl;
			exit(EXIT_FAILURE);
		}
		std::clog << "GLFW Initialized!\n" << std::endl;
		Contexts = (Engine::Graphics::GLContext**)malloc(sizeof(Engine::Graphics::GLContext*));
		//The first instance on the main thread cannot have sharing
		Contexts[0] = new Engine::Graphics::GLContext(this, info, nullptr);
		context_count = 1;
		context_size = 1;
		current = 0;
		display = Contexts[0]->display.Basic_Info;
		projection = Contexts[0]->projection;
	}
	else {
		main_thread_comm =
			connect<GLContextManager>(mainthreads, ContextManagerConnection,
				TYPE_CONTEXT_SERVER, TYPE_CONTEXT_CLIENT, 5, 5);
	}
	++instance_count;
}

inline GLContextManager::~GLContextManager() {
	//This destructor can only be safely called from the main thread.
	//other threads may need to be migrated to main thread first.
	if (!id) { 
		while (instance_count != 1); 
		for (size_t i = 0; i < context_count; ++i) {
			delete Contexts[i];
		}
	}
	else {
		for (size_t i = 0; i < context_count; ++i) {
			main_threads->post_event();
			//move contexts to main thread
			//delete Contexts[i];
		}
	}
	for (size_t i = 0; i < Context_recycled; ++i) {
		delete Recycled_context[i];
	}
	free(Recycled_context);
	free(Contexts);
	if (instance_count.fetch_sub(1) == 1) {
		glfwTerminate();
	}
}

inline size_t GLContextManager::GenContext(Engine::Graphics::WindowInfo & info, size_t shared_ID) {
	assert(id == 0);
	//Invalid parameters
	if (!info.Basic_Info.fullscreen && ((!info.Basic_Info.width) || (!info.Basic_Info.height))) return -1;
	Engine::Graphics::GLContext* shared = nullptr;
	if ((shared_ID == size_t(-1) && current != size_t(-1))
		|| shared_ID == current) shared = Contexts[current];
	else {
		if (!Contexts || !Contexts[shared_ID]) return -1;
		shared = Contexts[shared_ID];
	}
	if (Context_recycled) {
		size_t rtn = Recycled_context[Context_recycled - 1]->index;
		Contexts[rtn] = (Engine::Graphics::GLContext*)Recycled_context[Context_recycled - 1];
		new(Recycled_context[Context_recycled - 1])Engine::Graphics::GLContext(this, &info, shared);
		Context_recycled--;
		context_count++;
		current = rtn;
		display = Contexts[rtn]->display.Basic_Info;
		projection = Contexts[rtn]->projection;
		return rtn;
	}
	if (context_count == context_size) {
		size_t new_size = size_t(context_size*1.618) + 1;
		Engine::Graphics::GLContext** new_cache
			= (Engine::Graphics::GLContext**)realloc(Contexts, new_size * sizeof(Engine::Graphics::GLContext*));
		if (!new_cache) return -1;
		Contexts = new_cache;
	}
	Contexts[context_count] = new Engine::Graphics::GLContext(this, &info, shared);
	current = context_count;
	display = Contexts[current]->display.Basic_Info;
	projection = Contexts[current]->projection;
	context_count++;
	return current;
}

inline bool GLContextManager::SetContext(size_t ID) {
	if (ID >= context_count || !Contexts[ID]) return false;
	if (ID == current) return true;
	if (current != size_t(-1))
		Contexts[current]->display.Basic_Info = display;
	if (Contexts[ID]->MakeCurrent()) {
		current = ID;
		display = Contexts[ID]->display.Basic_Info;
		projection = Contexts[ID]->projection;
		return true;
	}
	return false;
}

inline bool GLContextManager::DelContext(size_t ID) {
	assert(id == 0);
	if (ID >= context_count) return false;
	//Cannot safely delete current one for now
	//(furthur calls to opengl will result in error)
	if (ID == current) {
		glfwMakeContextCurrent(nullptr);
		current = -1;
	}
	if (Contexts[ID]) {
		if (Context_recycled == Context_recycle_size) {
			size_t new_size = size_t(Context_recycle_size*1.618) + 1;
			recycled_GLContext** new_cache
				= (recycled_GLContext**)realloc(Recycled_context, new_size * sizeof(recycled_GLContext*));
			if (!new_cache) return false;
			Recycled_context = new_cache;
		}
		Recycled_context[Context_recycled] = (recycled_GLContext*)Contexts[ID];
		++Context_recycled;
		Contexts[ID]->~GLContext();
		Recycled_context[Context_recycled]->index = ID;
		Contexts[ID] = nullptr;
		context_count--;
		return true;
	}
	return false;
}

inline bool GLContextManager::reserve(size_t size) {
	if (size <= context_count) return true;
	Engine::Graphics::GLContext** new_cache
		= (Engine::Graphics::GLContext**)realloc(Contexts, size * sizeof(Engine::Graphics::GLContext*));
	if (!new_cache) {
		return false;
	}
	Contexts = new_cache;
	return true;
}

inline bool GLContextManager::EnableGLDebug(GLDEBUGPROC debugCallback) {
	if (current == size_t(-1)) return false;
	glEnable(GL_DEBUG_OUTPUT);
	if (!debugCallback) {
		debugCallback = Engine::Graphics::Contexts::Default::debugCallback;
	}
	glDebugMessageCallback(debugCallback, Contexts[current]);
	return true;
}
