#include <iostream>

#include "..\Engine\Core\Init\Global.h"
#include "..\Engine\Core\Init\Display.h"

using namespace Global;

int main(int argc, char* argv[], char* env[]) {
	argc_m = argc;
	argv_m = argv;
	env_m = env;
	Init();
	while(1){
		glfwWaitEvents();
	}
	system("pause");
	CleanUp();
	return 0;
}



