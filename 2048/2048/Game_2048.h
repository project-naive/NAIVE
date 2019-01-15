#pragma once
#include "..\Engine\Core\Game.h"


class Game_2048:
	public Engine::Core::Game {
public:
	Game_2048(const Engine::Graphics::Context::WindowInfo& info);
	~Game_2048();
	void notifyRefresh(GLFWwindow* window) {
		state->Draw();
	}
	void MainLoop() override {
		glfwPollEvents();
		while(running){
			state->Loop();
			running=state->running;
		}
	}
//	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
