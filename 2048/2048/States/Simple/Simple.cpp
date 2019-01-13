#include "Simple.h"

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include "..\..\..\Engine\Rendering\Models\Triangle\Triangle.h"

namespace States {
	Simple::Simple(Game_2048* parent) {
		this->parent = parent;
		TriangleModel = new Engine::Rendering::Models::Triangle(this);
	}
	void Simple::Init() {

	}
	void Simple::Destroy() {
		delete TriangleModel;
		TriangleModel = nullptr;
	}
	void Simple::Draw() {
		TriangleModel->VertexData[0] = {
			glm::vec3(0.25, -0.25, 0.0),
			glm::vec4(1, 0, 0, 1)
		};
		TriangleModel->VertexData[1] = {
			glm::vec3(-0.25, -0.25, 0.0),
			glm::vec4(0, 1, 0, 1)
		};
		TriangleModel->VertexData[2] = {
			glm::vec3(0.25, 0.25, 0.0),
			glm::vec4(0, 0, 1, 1)
		};
		TriangleModel->Draw();
		glfwSwapBuffers(parent->display.Window);
	}
	void Simple::Update() {
		TriangleModel->Draw();
		glfwSwapBuffers(parent->display.Window);
	}
	void Simple::Loop() {}
}

