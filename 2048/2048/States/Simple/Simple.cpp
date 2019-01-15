#include "Simple.h"

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include "..\..\..\Engine\Graphics\Models\Triangle\Triangle.h"

#include <iostream>
#include <fstream>

namespace States {
	Simple::Simple(Engine::Core::GlobalManagers& given_managers): State(given_managers) {
		TriangleModel = new Engine::Graphics::Models::Triangle(managers.ShaderManager);
		std::ifstream ifs;
		ifs.open(".\\Resources\\fonts\\arial.ttf");
		size_t size = ifs.seekg(0, std::ios::end).tellg();
		char* buffer=new char[size + 1];
		ifs.seekg(0);
		ifs.read(buffer, size);
		managers.TextManager.loadFace(buffer, size);
		managers.TextManager.genCharTexture(0, 128, 48);
		managers.TextManager.unloadFace();
		delete[] buffer;
	}

	Simple::~Simple() {
		delete TriangleModel;
	}
	void Simple::Draw() {
		TriangleModel->Begin();
		TriangleModel->Draw();
		const char* text = "Hello World!";
		uint32_t* chars = new uint32_t[strlen(text) + 1];
		for(size_t i = 0; i < strlen(text); i++){
			chars[i] = text[i];
		}
		chars[strlen(text)]=0;
		managers.TextManager.renderText(chars, glm::vec2(0.0f, 0.0f), glm::vec4(1.0f, 1.0f, 1.0f,1.0f));
		managers.ContextManager.Refresh();
	}
	void Simple::Update() {
		TriangleModel->VertexData[0] = {
			glm::vec3(0.5, -0.5, 0.0),
			glm::vec4(1, 0, 0, 1)
		};
		TriangleModel->VertexData[1] = {
			glm::vec3(-0.5, -0.5, 0.0),
			glm::vec4(0, 1, 0, 1)
		};
		TriangleModel->VertexData[2] = {
			glm::vec3(0.5, 0.5, 0.0),
			glm::vec4(0, 0, 1, 1)
		};
		TriangleModel->Update();
	}
	void Simple::Loop() {
		while(running){
			Update();
			Draw();
			glfwWaitEvents();
		}
	}
}

