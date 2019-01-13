#include "Game_2048.h"

#include "States/Simple/Simple.h"


Game_2048::Game_2048(const Engine::Core::WindowInfo& info): Game(info) {
	state = new States::Simple(this);
//	glfwSetKeyCallback(display.Window, KeyCallback);
}

Game_2048::~Game_2048() {

}

//void Game_2048::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
//
//}
