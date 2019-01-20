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
namespace States {
	State_2048::State_2048(Engine::Core::GlobalManagers & given_managers):
		State(given_managers) {
		size_t width, height;
		given_managers.ContextManager.GetCurrentDefaultResolution(width, height);
		glGenFramebuffers(1, &Board_FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, Board_FBO);
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2 * width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
		GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers);
		glClearColor(0xC6 / 255.0f, 0xC3 / 255.0f, 0xBA / 255.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		BlockModel = new ::State_2048::Models::Block(
			given_managers.ShaderManager, 
			given_managers.ShaderManager.addGeneric(
				*( new ::State_2048::Shaders::Block(given_managers.ShaderManager) )));
		BlockModel->data.Block_size = glm::vec2(0.24f, 0.24f);
		BlockModel->data.color = glm::vec4(0xE4 / 255.0f, 0xE2 / 255.0f, 0xD8 / 255.0f, 1.0f);
		BlockModel->data.radius = 0.2f;
		BlockModel->data.projection = glm::identity<glm::mat4>();
		BlockModel->Begin();
		glViewport(0, 0, width, height);
		BlockModel->data.Block_center = glm::vec2(-0.75f, -0.75f);
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				BlockModel->Update();
				BlockModel->Draw();
				BlockModel->data.Block_center += glm::vec2(0.5f, 0.0f);
			}
			BlockModel->data.Block_center += glm::vec2(-2.0f, 0.5f);
		}
		glViewport(width, 0, width, height);
		BlockModel->data.Block_center = glm::vec2(-0.75f, -0.75f);
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				BlockModel->data.color = number_colors[i][j];
				BlockModel->Update();
				BlockModel->Draw();
				BlockModel->data.Block_center += glm::vec2(0.5f, 0.0f);
			}
			BlockModel->data.Block_center += glm::vec2(-2.0f, 0.5f);
		}
		size_t num = 1;
		float x = -0.75f, y = -0.75f;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				num <<= 0x01;
				std::ostringstream oss;
				oss << num;
				float h = Managers.TextManager.GetHeight();
				float l = Managers.TextManager.TextLength(oss.str().c_str());
				float scale = 2.5f;
				if (num >= 64) scale = 2.0f;
				if (num >= 8196) scale = 1.5f;
				float x_pos = ( 1.0f + x )*( width / 2.0f ) - ( l / 2.0f )*scale;
				float y_pos = ( 1.0f + y )*( height / 2.0f ) - ( h / 2.0f )*scale* (0.8f);
				glm::vec4 color;
				if (num < 16)color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
				else color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				Managers.TextManager.renderText(oss.str().c_str(), glm::vec2(x_pos, y_pos), color, scale);
				x += 0.5f;
			}
			x -= 2.0f;
			y += 0.5f;
		}
		TextureModel = new ::State_2048::Models::TextureQuad(
			Managers.ShaderManager,
			Managers.ShaderManager.addGeneric(
				*(new ::State_2048::Shaders::TextureQuad(Managers.ShaderManager))));
		TextureModel->data.textureID = texture;
		TextureModel->data.projection = glm::identity<glm::mat4>();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		Managers.ContextManager.GetCurrentResolution(width, height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		TextureModel->data.tex_ori = glm::vec2(0.0f, 0.0f);
		TextureModel->data.tex_vec1 = glm::vec2(0.5f, 0.0f);
		TextureModel->data.tex_vec2 = glm::vec2(0.0f, 1.0f);
		TextureModel->data.pos = glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f);
		TextureModel->data.vec1 = glm::vec3(2.0f, 0.0f, 0.0f);
		TextureModel->data.vec2 = glm::vec3(0.0f, 2.0f, 0.0f);
		TextureModel->Begin();
		TextureModel->Update();
		TextureModel->Draw();
		std::srand(std::chrono::system_clock::now().time_since_epoch().count());
		std::rand();
	}

	void State_2048::Loop() {
		while (running) {
			Update();
			Draw();
			glfwPollEvents();
		}
	}

	void State_2048::Draw() {
		size_t width, height;
		Managers.ContextManager.GetCurrentResolution(width, height);
//		glBlitFramebuffer(0, 0, 2048, 1024, 0, 0, width, height, GL_COLOR_BUFFER_BIT,GL_LINEAR);
		TextureModel->Begin();
		TextureModel->Draw();
		if (std::chrono::steady_clock::now() - FPS_timer >= std::chrono::milliseconds(250)) {
			FPS = FPS_cache;
			FPS_timer = std::chrono::steady_clock::now();
			std::cout << "FPS:  " << FPS << std::endl;
		}
		glFlush();
		//		Managers.ContextManager.Refresh();
	};
	void State_2048::Update() {
		std::chrono::high_resolution_clock::time_point cur = std::chrono::high_resolution_clock::now();
		FPS_cache = 1 / std::chrono::duration<float>(std::chrono::duration_cast<std::chrono::duration<float>>( ( cur - last_update_time ) )).count();
		last_update_time = std::chrono::high_resolution_clock::now();
		char input = rand() % 4;
		switch (input) {
			case 0:
				break;
			case 1:
				break;
			case 2:
				break;
			case 3:
				break;
		}


	};
}


