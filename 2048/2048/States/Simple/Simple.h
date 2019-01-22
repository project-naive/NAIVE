#pragma once

#include "..\..\Game_2048.h"

#include <Graphics\Models\Text\Text.h>
#include <Graphics\Models\Triangle\Triangle.h>

#include "..\State_2048\Models\Fan\Fan.h"
#include "..\Simple\Models\TextureQuad\TextureQuad.h"
#include "..\Simple\Models\TexturedCube\TextureCube.h"

namespace States {
	class Simple:
		public Engine::Core::State{
	public:
		Simple(Engine::Core::GlobalManagers& given_managers);
		void Destroy() override final;
		void Draw() override final;
		void Update() override final;
		void Loop() override final;
	private:
		void notifyKey(int key, int scancode, int action, int mods, size_t contextID = 0) override final{}
		bool notifyContextClose(size_t ID) override final;
		const char* text = "Hello World!";
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

		struct PrivateModels {
			PrivateModels() = delete;
			PrivateModels(Engine::Graphics::Managers::Shader& SManager):
				TriangleModel(SManager),
				FanModel(SManager,
						 SManager.addGeneric(*( new ::State_2048::Shaders::Fan(SManager)))) {};
			Engine::Graphics::Models::Triangle TriangleModel;
			::State_2048::Models::Fan FanModel;

		};
		::State_2048::Models::TextureQuad* TextureModel;
		::State_2048::Models::TextureCube* CubeModel;
		PrivateModels PrivateModels;

		size_t ContextIDs[2];
		size_t context_count = 2;

		float updateFPS;
		float lowerFPS;
		std::chrono::high_resolution_clock::time_point update_start = std::chrono::high_resolution_clock::now();
		bool is_refresh_frame = false;
	};
}
