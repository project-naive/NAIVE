#include "Game_2048.h"

#include "States/Simple/Simple.h"
#include "States/Loading/Loading.h"
#include "States/Welcome/Welcome.h"


Game_2048::Game_2048(Engine::Graphics::WindowInfo& info) : Game(info) {
//	state = new States::Loading(Managers);
	state = new States::Welcome(Managers);
//	state = new States::Simple(Managers);
//	glfwSetKeyCallback(display.Window, KeyCallback);
}

Game_2048::~Game_2048() {
}

void Game_2048::MainLoop() {
	glfwPollEvents();
	while (state->running) {
		state->Loop();
	}
	if (running) {
		delete state;
		state = new ::States::State_2048(Managers);
		while (running) {
			state->Loop();
			running &= state->running;
		}
	}
}

//void Game_2048::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
//
//}
