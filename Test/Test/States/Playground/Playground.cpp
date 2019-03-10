#include "Playground.h"

namespace States {
	Playground::Playground(Engine::Core::GlobalManagers & given_managers):
		State(given_managers) {

	}

	void Playground::Loop() {
		glfwWaitEvents();
	}

	bool Playground::notifyContextClose(size_t ID) {
		running = false;
		return false;
	}
}


