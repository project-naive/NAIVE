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
		void notifyKey(int key, int scancode, int action, int mods, size_t contextID = 0) override final {
			if (action == GLFW_PRESS) {
				switch (key) {
					case GLFW_KEY_S: case GLFW_KEY_DOWN:
						last_input = 'S';
						input_got = true;
						break;
					case GLFW_KEY_A: case GLFW_KEY_LEFT:
						last_input = 'A';
						input_got = true;
						break;
					case GLFW_KEY_D: case GLFW_KEY_RIGHT:
						last_input = 'D';
						input_got = true;
						break;
					case GLFW_KEY_W: case GLFW_KEY_UP:
						last_input = 'W';
						input_got = true;
						break;
					case GLFW_KEY_Q:
						quit_key = true;
					default:
						break;
				}
			}
		}
		GLuint Board_FBO = 0;
		GLuint texture = 0;
		::State_2048::Models::Block* BlockModel;
		::State_2048::Models::TextureQuad* TextureModel;
		float FPS_cache = 0;
		float FPS = 0;
		std::chrono::steady_clock::time_point FPS_timer;

		unsigned char Board[4][4]{};

		bool move_left();
		bool move_right();
		bool move_up();
		bool move_down();

		bool can_continue = true;
		unsigned char last_input = 0;
		bool input_got = false;
		bool valid_move = false;
		bool quit_key = false;

		void clearBoard(int x0, int x1, int y0, int y1);
		void drawBlock(unsigned char block, float Xc, float Yc);
	};
}