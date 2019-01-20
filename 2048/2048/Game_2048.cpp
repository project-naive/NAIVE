#include "Game_2048.h"

#include "States/Simple/Simple.h"
#include "States/Loading/Loading.h"


Game_2048::Game_2048(Engine::Graphics::WindowInfo& info) : Game(info) {
//	state = new States::Loading(Managers);
	state = new States::State_2048(Managers);
//	glfwSetKeyCallback(display.Window, KeyCallback);
}

Game_2048::~Game_2048() {
}

//void Game_2048::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
//
//}
