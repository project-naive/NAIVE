#include "State_2048.h"

#include <glm\gtc\matrix_transform.hpp>
#include <iostream>
#include <sstream>

const glm::vec4 number_colors[4][4] = {
	{
		glm::vec4(0xF7 / 255.0f, 0xF2 / 255.0f, 0xE0 / 255.0f, 1.0f),
		glm::vec4(0xF5 / 255.0f, 0xEC / 255.0f, 0xCE / 255.0f, 1.0f),
		glm::vec4(0xF5 / 255.0f, 0xD0 / 255.0f, 0xA9 / 255.0f, 1.0f),
		glm::vec4(0xF7 / 255.0f, 0x7F / 255.0f, 0x81 / 255.0f, 1.0f),
	},
    {
		glm::vec4(0xF7 / 255.0f, 0x9F / 255.0f, 0x81 / 255.0f, 1.0f),
		glm::vec4(0xFE / 255.0f, 0x64 / 255.0f, 0x2E / 255.0f, 1.0f),
		glm::vec4(0xF7 / 255.0f, 0xD3 / 255.0f, 0x58 / 255.0f, 1.0f),
		glm::vec4(0xF7 / 255.0f, 0xD3 / 255.0f, 0x58 / 255.0f, 1.0f),
	},
    {
		glm::vec4(0xF7 / 255.0f, 0xD3 / 255.0f, 0x58 / 255.0f, 1.0f),
		glm::vec4(0xF7 / 255.0f, 0xD3 / 255.0f, 0x58 / 255.0f, 1.0f),
		glm::vec4(0xF7 / 255.0f, 0xD3 / 255.0f, 0x58 / 255.0f, 1.0f),
		glm::vec4(0x01 / 255.0f, 0xDF / 255.0f, 0xA5 / 255.0f, 1.0f),
	},
    {
		glm::vec4(0x01 / 255.0f, 0xDF / 255.0f, 0x74 / 255.0f, 1.0f),
		glm::vec4(0x08 / 255.0f, 0x8A / 255.0f, 0x4B / 255.0f, 1.0f),
		glm::vec4(0x00 / 255.0f, 0xBF / 255.0f, 0xFF / 255.0f, 1.0f),
		glm::vec4(0x00 / 255.0f, 0x00 / 255.0f, 0xFF / 255.0f, 1.0f),
	}
};
//		glClearColor(0xC6/255.0f, 0xC3/255.0f, 0xBA/255.0f, 1.0f);
namespace States {
	State_2048::State_2048(Engine::Core::GlobalManagers & given_managers):
		State(given_managers) {
		BlockModel = new ::State_2048::Models::Block(
			given_managers.ShaderManager, 
			given_managers.ShaderManager.addGeneric(
				*( new ::State_2048::Shaders::Block(given_managers.ShaderManager) )));
		BlockModel->data.Block_size = glm::vec2(0.24f, 0.24f);
		BlockModel->data.color = glm::vec4(0xE4 / 255.0f, 0xE2 / 255.0f, 0xD8 / 255.0f, 1.0f);
		BlockModel->data.radius = 0.2f;
		BlockModel->data.projection = glm::identity<glm::mat4>();
		BlockModel->data.Block_center = glm::vec2(0.0f, 0.0f);
		BlockModel->Update();
	}

	void State_2048::Loop() {
		while (running) {
			Update();
			Draw();
			glfwPollEvents();
		}
	}

	void State_2048::Draw() {
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		BlockModel->Begin();
		BlockModel->Update();
		BlockModel->Draw();
		if (std::chrono::steady_clock::now() - FPS_timer >= std::chrono::milliseconds(250)) {
			FPS = FPS_cache;
			FPS_timer = std::chrono::steady_clock::now();
		}
		std::ostringstream oss;
		oss << "FPS:  " << FPS;
		Managers.TextManager.renderText(oss.str().c_str(), glm::vec2(40.0f, 900.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 2);
		Managers.ContextManager.Refresh();
	};
	void State_2048::Update() {
		std::chrono::high_resolution_clock::time_point cur = std::chrono::high_resolution_clock::now();
		FPS_cache = 1 / std::chrono::duration<float>(std::chrono::duration_cast<std::chrono::duration<float>>( ( cur - last_update_time ) )).count();
		last_update_time = std::chrono::high_resolution_clock::now();
	};
}


