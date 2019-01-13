#include <iostream>
#include <fstream>
#include <vector>

#include "..\Engine\Core\Init\Global.h"
#include "..\Engine\Core\Init\Display.h"

#include "..\Engine\Rendering\Managers\TextManager.h"
#include "..\Engine\Rendering\Managers\VertexAttributeManager.h"
#include "..\Engine\Rendering\Managers\ShaderManager.h"

#include "..\Engine\Rendering\Models\Triangle\Triangle.h"

using namespace Global;

int main(int argc, char* argv[], char* env[]) {
	argc_m = argc;
	argv_m = argv;
	env_m = env;
	Init();
//	Rendering::Manager::Text TextManager;
	Rendering::Manager::Shader ShaderManager;
	Rendering::Manager::VertexAttribute Vertexes;
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
	Rendering::Models::Triangle Triangle_Model;
	Triangle_Model.VertexData[0] = {
		glm::vec3(-1.0f,1.0f,1.0f),
		glm::vec4(1.0f,1.0f,1.0f,1.0f) 
	};
	Triangle_Model.VertexData[1] = {
		glm::vec3(1.0f,-1.0f,1.0f),
		glm::vec4(1.0f,1.0f,1.0f,1.0f)
	};
	Triangle_Model.VertexData[2] = {
		glm::vec3(-1.0f,-1.0f,1.0f),
	    glm::vec4(1.0f,1.0f,1.0f,1.0f)
	};
	glfwPollEvents();
	while(1){
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		Triangle_Model.Draw();
		glfwSwapBuffers(Display::mainWindow);
		glfwWaitEvents();
	}
	system("pause");
	CleanUp();
	return 0;
}



