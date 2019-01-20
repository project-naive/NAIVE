#pragma once

#include "..\..\Game_2048.h"

#include "Models\Block\Block.h"
#include "..\Simple\Models\TextureQuad\TextureQuad.h"

namespace States {
	class State_2048:
		public Engine::Core::State{
	public:
		State_2048(Engine::Core::GlobalManagers& given_managers);
		void Destroy() override final {};
		void Draw() override final;
		void Update() override final;
		void Loop() override final;
		bool notifyContextClose(size_t ID) override final {
			running = false;
			return false;
		};
		GLuint Board_FBO = 0;
		GLuint texture = 0;
		::State_2048::Models::Block* BlockModel;
		::State_2048::Models::TextureQuad* TextureModel;
		float FPS_cache = 0;
		float FPS = 0;
		std::chrono::steady_clock::time_point FPS_timer;

		unsigned char Board[4][4]{};

	};
}