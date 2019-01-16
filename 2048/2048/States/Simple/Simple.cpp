#include "Simple.h"

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <glm\gtc\matrix_transform.hpp>

namespace States {
	Simple::Simple(Engine::Core::GlobalManagers& given_managers): 
		State(given_managers) {
		Main_context = Managers.ContextManager.GetContext();
		TriangleModel = new Engine::Graphics::Models::Triangle(Managers.ShaderManager);
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
		FanModel=new ::State_2048::Models::Fan(Managers.ShaderManager);
		FanModel->data=::State_2048::Models::Fan::DataFormat{
			glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
			glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
			glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
			glm::vec4(480.0f, 60.0f, 0.0f, 1.0f),
			glm::vec4(-64.0f, 15.0f, 0.0f, 0.0f),
			glm::vec4(30.0f, 64.0f, 0.0f, 0.0f),
		};
		Managers.ContextManager.GetCurrentDefaultResolution(rb_width, rb_height);
		projection = glm::ortho(0.0f, float(rb_width), 0.0f, float(rb_height));
		FanModel->projection = projection;
		chars = new uint32_t[strlen(text)];
		for (size_t i = 0; i < strlen(text); i++) {
			chars[i] = text[i];
		}
		glGenFramebuffers(1, &texture_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, texture_fbo);
		glViewport(0, 0, GLsizei(rb_width), GLsizei(rb_height));
		glGenBuffers(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GLsizei(rb_width), GLsizei(rb_height), 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glGenRenderbuffers(1, &texture_rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, texture_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32F, GLsizei(rb_width), GLsizei(rb_height));
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, texture_rbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers);
		Engine::Graphics::Context::WindowInfo info=Engine::Graphics::Contexts::Default::default_window();
		info.Basic_Info.height = 512;
		info.Basic_Info.width = 512;
		info.Extended_Info.GL_Version_Major = 4;
		info.Extended_Info.GL_Version_Minor = 4;
		info.Extended_Info.title = "Another Window!";
		Sub_context = Managers.ContextManager.GenContext(info, Managers.ContextManager.GetContext());
		Managers.ContextManager.SetContext(Main_context);

		FPS_timer = std::chrono::steady_clock::now();
	}

	void Simple::Destroy(){
		glDeleteFramebuffers(1, &texture_fbo);
		delete TriangleModel;
		delete FanModel;
		delete[] chars;
	}

	void Simple::Draw() {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, texture_fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		TriangleModel->Begin();
		TriangleModel->Update();
		TriangleModel->Draw();
		FanModel->Begin();
		FanModel->Update();
		FanModel->Draw();
		Managers.TextManager.renderText(chars, glm::vec2(0.0f, 0.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), strlen(text));
		std::chrono::steady_clock::time_point cur = std::chrono::steady_clock::now();
		if (cur - FPS_timer >= std::chrono::milliseconds(500)) {
			FPS_timer = cur;
			FPS = FPS_cache;
		}
		std::ostringstream oss;
		oss << "FPS:  " << FPS;
		Managers.TextManager.renderText(oss.str().c_str(), glm::vec2(20.0f, 450.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		size_t width, height;
		Managers.ContextManager.GetCurrentResolution(width, height);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, texture_fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, GLint(rb_width), GLint(rb_height), 0, 0, GLint(width), GLint(height), GL_COLOR_BUFFER_BIT, GL_LINEAR);




		Managers.ContextManager.Refresh();
	}
	void Simple::Update() {
		std::chrono::high_resolution_clock::time_point cur = std::chrono::high_resolution_clock::now();
		FPS_cache = 1 / std::chrono::duration<float>(std::chrono::duration_cast<std::chrono::duration<float>>((cur - last_update_time))).count();
		last_update_time = std::chrono::high_resolution_clock::now();
		//Do other things that modifies data here.
	}

	void Simple::Loop() {
		while(running){
			Update();
			Draw();
			glfwWaitEvents();
		}
	}

	void Simple::notifyContextClose(size_t ID) {
		if (ID == Main_context) {
			Managers.ContextManager.DelContext(ID);
			running = false;
		}
		else {
			Managers.ContextManager.DelContext(ID);
		}
	}
}

