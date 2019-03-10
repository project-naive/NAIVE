/*
*	The global variables throughout the game, used only where
*	state changes do not affect behavior
*/

#pragma once

#include "..\State.h"

namespace Global {
	//The arguments called when the game is launched
	extern int argc_m;
	extern char** argv_m;
	extern char** env_m;

	//Title of the game as shown on window bar
	extern const char* game_title;

	//Whether the game window is not under focus
	extern bool idle;

	//The functions for initializing and cleaning up on start and end
	void Init();
	void CleanUp();

	//The error callback for the glfw instance (whole program's intance)
	namespace Callback {
		void error(int err_code, const char* info);
	}
}

