#include <iostream>
#include <fstream>
#include <vector>

#include "Test.h"

int argc_m;
char** argv_m;
char** env_m;

int main(int argc, char* argv[], char* env[]) {
	argc_m = argc;
	argv_m = argv;
	env_m = env;
	Engine::Graphics::WindowInfo window{ Engine::Graphics::Contexts::Default::default_window() };
	window.Basic_Info.width = 800;
	window.Basic_Info.height = 600;
	window.Extended_Info.Display.Window.title = "Test";
	window.Extended_Info.Context.ContextInfo.GL_Version_Major = 4;
	window.Extended_Info.Context.ContextInfo.GL_Version_Minor = 4;
	window.Extended_Info.Display.SwapInterval = 0;
	window.Extended_Info.Context.ContextInfo.debug = true;
	window.Extended_Info.Context.FramebufferInfo.double_buffer = true;
	window.Extended_Info.Display.Window.CreationInfo.minWidth = 0;
	window.Extended_Info.Display.Window.CreationInfo.minHeight = 0;
	window.Extended_Info.Display.Window.CreationInfo.maxWidth = INT_MAX;
	window.Extended_Info.Display.Window.CreationInfo.maxHeight = INT_MAX;
	{
		Test this_game{ window };
		this_game.MainLoop();
	}
//	system("pause");
	return 0;
}
