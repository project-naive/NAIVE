#include "Simple.h"

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include "..\..\..\Engine\Graphics\Models\Triangle\Triangle.h"

namespace States {
	Simple::Simple(Engine::Core::GlobalManagers& given_managers): State(given_managers) {
		TriangleModel = new Engine::Graphics::Models::Triangle(managers.ShaderManager);
	}

	Simple::~Simple() {
		delete TriangleModel;
	}
	void Simple::Draw() {
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
		TriangleModel->Begin();
		TriangleModel->Draw();
		managers.ContextManager.Refresh();
	}
	void Simple::Update() {
		TriangleModel->Draw();
		managers.ContextManager.Refresh();
	}
	void Simple::Loop() {}
}

