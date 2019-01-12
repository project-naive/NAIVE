#include <iostream>
#include <fstream>
#include <vector>

#include "..\Engine\Core\Init\Global.h"
#include "..\Engine\Core\Init\Display.h"

#include "..\Engine\Rendering\Managers\Text.h"

Rendering::Manager::Text TextManager;

using namespace Global;

int main(int argc, char* argv[], char* env[]) {
	argc_m = argc;
	argv_m = argv;
	env_m = env;
	Init();
	std::ifstream ifs;
	ifs.open("arial.ttf");
	ifs.seekg(0, std::ios::end);
	size_t size = size_t(ifs.tellg());
	std::vector<char> str(size);
	ifs.seekg(0);
	ifs.read(&str[0], size);
	ifs.close();
	size_t index = TextManager.loadFace(str.data(), size);
	TextManager.genCharTexture(0, 128, 48);

	while(1){
		glfwWaitEvents();
	}
	system("pause");
	CleanUp();
	return 0;
}



