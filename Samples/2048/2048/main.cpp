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
	window.Extended_Info.Display.Window.title = "2048";
	window.Extended_Info.Display.Window.CreationInfo.minWidth = 0;
	window.Extended_Info.Display.Window.CreationInfo.minHeight = 0;
	window.Extended_Info.Display.Window.CreationInfo.maxWidth = INT_MAX;
	window.Extended_Info.Display.Window.CreationInfo.maxHeight = INT_MAX;
	window.Extended_Info.Context.ContextInfo.GL_Version_Major = 3;
	window.Extended_Info.Context.ContextInfo.GL_Version_Minor = 3;
	window.Extended_Info.Display.SwapInterval = 1;
	window.Extended_Info.Context.ContextInfo.debug = false;
	window.Extended_Info.Context.FramebufferInfo.double_buffer = false;
	{
		Game_2048 this_game{ window };
		this_game.MainLoop();
	}
//	system("pause");
	return 0;
}



