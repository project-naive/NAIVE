#pragma once
#include "..\Engine\Core\Game.h"


class Game_2048:
	public Engine::Core::Game {
public:
	Game_2048(const Engine::Core::WindowInfo& info);
	~Game_2048();
	static void Refresh(GLFWwindow* window) {
		state->Draw();
	}
//	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
