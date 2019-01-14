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
	Engine::Core::WindowInfo window{ Engine::Core::Default::default_window() };
	window.width = 512;
	window.height = 512;
	window.title = "2048";
	window.Refresh = Game_2048::Refresh;
	Game_2048 this_game{ window };
//	Init();
//	Graphics::Managers::Text TextManager;
//	Engine::Graphics::Managers::Shader ShaderManager;
//	Engine::Graphics::Managers::VertexAttribute Vertexes;
//	std::ifstream ifs;
//	ifs.open(".\\Resources\\fonts\\arial.ttf");
//	ifs.seekg(0, std::ios::end);
//	size_t size = size_t(ifs.tellg());
//	std::vector<char> str(size);
//	ifs.seekg(0);
//	ifs.read(&str[0], size);
//	ifs.close();
//	size_t index = TextManager.loadFace(str.data(), size);
//	TextManager.genCharTexture(0, 128, 48);
	glfwPollEvents();
	while(1){
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		this_game.state->Draw();
		glfwPollEvents();
	}
	system("pause");
//	CleanUp();
	return 0;
}



