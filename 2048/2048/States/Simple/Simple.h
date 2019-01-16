#pragma once

#include "..\..\Game_2048.h"

#include <Graphics\Models\Text\Text.h>
#include <Graphics\Models\Triangle\Triangle.h>

#include "..\State_2048\Models\Fan\Fan.h"

namespace States {
	class Simple:
		public Engine::Core::State{
	public:
		Simple(Engine::Core::GlobalManagers& given_managers);
		Engine::Graphics::Models::Triangle* TriangleModel;
		::State_2048::Models::Fan* FanModel;
		void Destroy() override;
		void Draw() override;
		void Update() override;
		void Loop() override;
		void notifyContextClose(size_t ID) override;
		const char* text = "Hello World!";
		uint32_t* chars = nullptr;
		float FPS_cache = 0;
		float FPS = 0;
		std::chrono::steady_clock::time_point FPS_timer;
		//The projection matrix for default size 
		//when coordinates are given in pixels
		glm::mat4 projection;
		GLuint texture_fbo;
		GLuint texture_rbo;
		GLuint texture;
		size_t rb_width, rb_height;

		size_t Main_context;
		size_t Sub_context;
	};
}
