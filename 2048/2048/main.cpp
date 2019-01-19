#include <iostream>
#include <fstream>
#include <vector>

#include "Game_2048.h"

int argc_m;
char** argv_m;
char** env_m;

int main(int argc, char* argv[], char* env[]) {
	argc_m = argc;
	argv_m = argv;
	env_m = env;
	Engine::Graphics::WindowInfo window{ Engine::Graphics::Contexts::Default::default_window() };
	window.Basic_Info.width = 1024;
	window.Basic_Info.height = 1024;
	window.Extended_Info.title = "2048";
	window.Extended_Info.GL_Version_Major = 4;
	window.Extended_Info.GL_Version_Minor = 4;
	window.Extended_Info.swap_interval = 0;
	window.Extended_Info.debug = true;
	window.Extended_Info.double_buffer = true;
	{
		Game_2048 this_game{ window };
		this_game.MainLoop();
	}
//	system("pause");
	return 0;
}



