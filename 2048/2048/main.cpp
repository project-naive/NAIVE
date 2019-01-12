#include <iostream>
#include <fstream>
#include <vector>

#include "..\Engine\Core\Init\Global.h"
#include "..\Engine\Core\Init\Display.h"

#include "..\Engine\Rendering\Managers\TextManager.h"

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
	size_t size = ifs.tellg();
	std::vector<unsigned char> str(size);
	ifs.seekg(0);
	ifs.read((char*)&str[0], size);
	ifs.close();
	size_t index = TextManager.fonts.load_face(str.data(), size);







	while(1){
		glfwWaitEvents();
	}
	system("pause");
	CleanUp();
	return 0;
}



