#include <iostream>
#include <fstream>
#include <vector>

//#include "..\Engine\Core\Init\Global.h"
//#include "..\Engine\Core\Init\Display.h"

#include "Game_2048.h"

int argc_m;
char** argv_m;
char** env_m;

int main(int argc, char* argv[], char* env[]) {
	argc_m = argc;
	argv_m = argv;
	env_m = env;
	Engine::Graphics::Context::WindowInfo window{ Engine::Graphics::Contexts::Default::default_window() };
	window.width = 512;
	window.height = 512;
	window.title = "2048";
	Game_2048 this_game{ window };
	this_game.MainLoop();
//	system("pause");
	return 0;
}



