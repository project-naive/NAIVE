#pragma once

#include <Core\Game.h>


class Game_2048:
	public Engine::Core::Game {
public:
	Game_2048(Engine::Graphics::Context::WindowInfo& info);
	~Game_2048();
	void notifyRefresh(GLFWwindow* window) {
		state->Draw();
	}
	void MainLoop() override {
		glfwPollEvents();
		while(running){
			state->Loop();
			running = state->running;
		}
		glfwTerminate();
	}

	void notifyClose(size_t contextID) override {
		state->notifyContextClose(contextID);
	 }
//	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
