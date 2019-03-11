#include "Simple.h"

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <glm\gtc\matrix_transform.hpp>

namespace States {
	Simple::Simple(Engine::Core::GlobalManagers& given_managers):
		State(given_managers),
		PrivateModels(given_managers.ShaderManager) {
		ContextIDs[0] = Managers.ContextManager.GetCurrent();
		PrivateModels.TriangleModel.VertexData[0] = {
			glm::vec3(0.5, -0.5, 0.0),
			glm::vec4(1, 0, 0, 1)
		};
		PrivateModels.TriangleModel.VertexData[1] = {
			glm::vec3(-0.5, -0.5, 0.0),
			glm::vec4(0, 1, 0, 1)
		};
		PrivateModels.TriangleModel.VertexData[2] = {
			glm::vec3(0.5, 0.5, 0.0),
			glm::vec4(0, 0, 1, 1)
		};
		PrivateModels.FanModel.data=::State_2048::Models::Fan::DataFormat{
			glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
			glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
			glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
			glm::vec4(960.0f, 120.0f, 0.0f, 1.0f),
			glm::vec3(-128.0f, 30.0f, 0.0f),
			glm::vec3(60.0f, 128.0f, 0.0f),
		};
		Managers.ContextManager.GetCurrentDefaultResolution(rb_width, rb_height);
		projection = Managers.ContextManager.GetProjection();
		PrivateModels.FanModel.projection = projection;
		Managers.TextManager.UpdateDisplay(projection);
		glGenFramebuffers(1, &texture_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, texture_fbo);
		glViewport(0, 0, GLsizei(rb_width), GLsizei(rb_height));
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GLsizei(rb_width), GLsizei(rb_height), 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//		glGenRenderbuffers(1, &texture_rbo);
//		glBindRenderbuffer(GL_RENDERBUFFER, texture_rbo);
//		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32F, GLsizei(rb_width), GLsizei(rb_height));
//		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, texture_rbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0  };
		glDrawBuffers(1, DrawBuffers);
		Engine::Graphics::WindowInfo info = Engine::Graphics::Contexts::Default::default_window();
		info.Basic_Info.height = 1024;
		info.Basic_Info.width = 1024;
		info.Extended_Info.Context.ContextInfo.GL_Version_Major = 4;
		info.Extended_Info.Context.ContextInfo.GL_Version_Minor = 4;
		info.Extended_Info.Display.Window.title = "Another Window!";
		info.Extended_Info.Display.SwapInterval = 0;
		info.Extended_Info.Context.ContextInfo.debug = true;
		info.Extended_Info.Context.FramebufferInfo.double_buffer = true;
	//	ContextIDs[1] = Managers.ContextManager.GenContext(info, Managers.ContextManager.GetCurrent());
		//Do stuff with the other context...
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		size_t texture_shader = given_managers.ShaderManager.addGeneric(*(new ::State_2048::Shaders::TextureQuad(given_managers.ShaderManager)));
		TextureModel = new 
			::State_2048::Models::TextureQuad(
				given_managers.ShaderManager, texture_shader);
		TextureModel->data = {
			glm::vec4(0.0f,0.0f,0.0f, 1.0f),
			glm::vec3(float(rb_width), 0.0f, 0.0f),
			glm::vec3(0.0f, float(rb_height), 0.0f),
			glm::vec2(0.0f,0.0f),
			glm::vec2(1.0f,0.0f),
			glm::vec2(0.0f,1.0f),
			texture,
			projection
		};
		CubeModel = new ::State_2048::Models::TextureCube(
			given_managers.ShaderManager, texture_shader
		);
		CubeModel->data = {
			glm::vec4(-256.0f, -256.0f, -256.0f, 1.0f),
			glm::vec3(512.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 512.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 512.0f),
			texture,
			::State_2048::Models::TextureCube::TextureFlagBits(0),
			projection
		};
		Managers.ContextManager.SetContext(ContextIDs[0]);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		FPS_timer = std::chrono::steady_clock::now();
	}

	void Simple::Destroy() {
		glDeleteFramebuffers(1, &texture_fbo);
		glDeleteRenderbuffers(1, &texture_rbo);
		glDeleteTextures(1, &texture);
	}

	void Simple::Draw() {
		Managers.ContextManager.SetContext(ContextIDs[0]);
		std::chrono::steady_clock::time_point cur = std::chrono::steady_clock::now();
		size_t width, height;
		if (is_refresh_frame) {
			updateFPS = 1 / std::chrono::duration<float>(std::chrono::duration_cast<std::chrono::duration<float>>( ( std::chrono::high_resolution_clock::now() - update_start ) )).count();
			is_refresh_frame = false;
		}
		if (cur - FPS_timer >= std::chrono::milliseconds(250)) {
			is_refresh_frame = true;
			update_start = std::chrono::high_resolution_clock::now();
			FPS_timer = cur;		
			Managers.ContextManager.SetContext(ContextIDs[0]);
			FPS = FPS_cache;
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, texture_fbo);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glViewport(0, 0, GLsizei(rb_width), GLsizei(rb_height));
			PrivateModels.TriangleModel.Begin();
			PrivateModels.TriangleModel.Draw();
			PrivateModels.FanModel.Begin();
			PrivateModels.FanModel.Draw();
			Managers.TextManager.renderText(text, glm::vec2(0.0f, 0.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.8f, strlen(text));
			std::ostringstream oss;
			oss << "FPS:  " << FPS;
			Managers.TextManager.renderText(oss.str().c_str(), glm::vec2(40.0f, 900.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),0.8f);
			oss.str("");
			oss << "Prev Frame Update FPS:  " << updateFPS;
			Managers.TextManager.renderText(oss.str().c_str(), glm::vec2(40.0f, 860.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.32f);
			oss.str("");
			oss << "FPS (Lower):  " << std::min(FPS, lowerFPS);
			lowerFPS = 10000;
			Managers.TextManager.renderText(oss.str().c_str(), glm::vec2(40.0f, 800.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.4f);
			//Do stuff with the rendered render buffer
			/*
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			Managers.ContextManager.GetCurrentResolution(width, height);
			glViewport(0, 0, GLsizei(width), GLsizei(height));
			glBlitFramebuffer(0, 0, GLint(rb_width), GLint(rb_height),
				0, 0, GLint(width), GLint(height),
				GL_COLOR_BUFFER_BIT, GL_LINEAR);
			Managers.ContextManager.SwapBuffers();
			*/
			/*
			Managers.ContextManager.GetCurrentResolution(width, height);
			glBlitFramebuffer(0, 0, GLint(rb_width), GLint(rb_height),
				0, 0, GLint(width), GLint(height),
				GL_COLOR_BUFFER_BIT, GL_LINEAR);
				*/
		}
	//	if(ContextIDs[1]!=size_t(-1)) {
	//		Managers.ContextManager.SetContext(ContextIDs[1]);
			Managers.ContextManager.GetCurrentResolution(width, height);
			if (is_refresh_frame) {
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glViewport(0, 0, GLsizei(width), GLsizei(height));
			}
			else {
				lowerFPS = std::min(FPS, lowerFPS);
			}
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			CubeModel->Begin();
			CubeModel->Draw();
//				TextureModel->Begin();
//				TextureModel->Draw();
			Managers.ContextManager.SwapBuffers();
	//	}
	}
	void Simple::Update() {
		std::chrono::high_resolution_clock::time_point cur = std::chrono::high_resolution_clock::now();
		FPS_cache = 1 / std::chrono::duration<float>(std::chrono::duration_cast<std::chrono::duration<float>>((cur - last_update_time))).count();
		last_update_time = std::chrono::high_resolution_clock::now();
		//Do other things that modifies data here.
		PrivateModels.TriangleModel.Update();
		PrivateModels.FanModel.Update();
		TextureModel->Update();
		float elapsed = std::chrono::duration<float>(std::chrono::duration_cast<std::chrono::duration<float>>( start_time - std::chrono::steady_clock::now() )).count();
		glm::mat4 ModelView = glm::mat4(1.0f);
		ModelView = glm::rotate(ModelView, 1.1f*elapsed, glm::vec3(1.0, 0.0, 0.0));
		ModelView = glm::rotate(ModelView, 1.2f*elapsed, glm::vec3(0.0, 1.0, 0.0));
		ModelView = glm::rotate(ModelView, 1.3f*elapsed, glm::vec3(0.0, 0.0, 1.0));
		ModelView = glm::scale(ModelView, glm::vec3(1 / 512.0f, 1 / 512.0f, 1 / 512.0f));
//		ModelView = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 1024.0f) * ModelView;
		CubeModel->data.projection =  ModelView;
		CubeModel->Update();
	}

	void Simple::Loop() {
		while(running){
			Update();
			Draw();
			glfwPollEvents();
		}
	}

	bool Simple::notifyContextClose(size_t ID) {
		if (ID == ContextIDs[0] || ID == ContextIDs[1]) {
			Managers.ContextManager.DelContext(ID);
			running = false;
			return false;
		}
		else {
			for (size_t i = 0; i < context_count; i++) {
				if (ID == ContextIDs[i]) {
					//Free resorces of this context
					ContextIDs[i] = -1;
					Managers.ContextManager.DelContext(ID);
					//Change to other context
					Managers.ContextManager.SetContext(ContextIDs[0]);
					return true;
				}
			}
			//The context is not managed by current state
			return false;
		}

	}
}

