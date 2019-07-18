#include "State_2048.h"

#include <glm\gtc\matrix_transform.hpp>
#include <iostream>
#include <sstream>

#include <MI/mimalloc.h>

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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GLsizei(2 * width), GLsizei(height), 0, GL_RGBA, GL_FLOAT, nullptr);
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
		BlockModel->data.Block_size = glm::vec2(1.0f, 1.0f);
		BlockModel->data.color = glm::vec4(0xC6 / 255.0f, 0xC3 / 255.0f, 0xBA / 255.0f, 1.0f);
		BlockModel->data.radius = 0.0f;
		BlockModel->data.projection = glm::identity<glm::mat4>();
		BlockModel->Begin();
		glViewport(0, 0, GLsizei(width), GLsizei(height));
		BlockModel->data.Block_center = glm::vec2(0.0f, 0.0f);
		BlockModel->Update();
		BlockModel->Draw();
		BlockModel->data.Block_size = glm::vec2(0.24f, 0.24f);
		BlockModel->data.color = glm::vec4(0xE4 / 255.0f, 0xE2 / 255.0f, 0xD8 / 255.0f, 1.0f);
		BlockModel->data.radius = 0.2f;
		BlockModel->data.Block_center = glm::vec2(-0.75f, -0.75f);
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				BlockModel->Update();
				BlockModel->Draw();
				BlockModel->data.Block_center += glm::vec2(0.5f, 0.0f);
			}
			BlockModel->data.Block_center += glm::vec2(-2.0f, 0.5f);
		}
		glViewport(GLint(width), 0, GLsizei(width), GLsizei(height));
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
				float scale = 1.0f;
				if (num > 64) scale = 0.8f;
				if (num >= 8196) scale = 0.6f;
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
		glViewport(0, 0, GLsizei(width), GLsizei(height));
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
		std::srand(std::chrono::duration_cast<std::chrono::duration<int>>(std::chrono::system_clock::now().time_since_epoch()).count());
		std::rand();
//		mi_collect(true);
	}

	void State_2048::Loop() {
		Update();
		while (running) {
			while (can_continue && running) {
				input_got = false;
				glfwWaitEvents();
				/*
				int input = rand() % 4;
				switch (input) {
					case 0:
						last_input = 'S';
						input_got = true;
						break;
					case 1:
						last_input = 'A';
						input_got = true;
						break;
					case 2:
						last_input = 'D';
						input_got = true;
						break;
					case 3:
						last_input = 'W';
						input_got = true;
						break;
				}*/
				if (input_got) {
					valid_move = false;
					Draw();
					if (valid_move) {
						Update();
						can_continue = false;
						for (int i = 0; i < 4; i++) {
							for (int j = 0; j < 4; j++) {
								if (!Board[i][j]) can_continue = true;
							}
						}
						for (int i = 0; i < 3; i++) {
							for (int j = 0; j < 4; j++) {
								if (Board[i][j] == Board[i + 1][j]) can_continue = true;
							}
						}
						for (int i = 0; i < 4; i++) {
							for (int j = 0; j < 3; j++) {
								if (Board[i][j] == Board[i][j + 1]) can_continue = true;
							}
						}
					}
				}
			}
			quit_key = false;
			renderExitText();
			glFlush();
			quit_key = false;
			while (!quit_key&&running) {
				glfwWaitEvents();
			}
			running = false;
		}

	}

	void State_2048::Draw() {
		switch (last_input) {
			case 'S':
				valid_move = move_down();
				break;
			case 'A':
				valid_move = move_left();
				break;
			case 'D':
				valid_move = move_right();
				break;
			case 'W':
				valid_move = move_up();
				break;
			default:
				break;
		}
	};
	void State_2048::Update() {
		bool okay = true;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				if (!Board[i][j]) okay = false;
			}
		}
		while (!okay) {
			int pos = std::rand();
			int x = pos % 4;
			int y = ( pos / 4 ) % 4;
			if (!Board[y][x]) {
				okay = true;
				int num = std::rand() % 2;
				Board[y][x] = num + 1;
				drawBlock(num + 1, x*0.5f - 0.75f, y*0.5f - 0.75f);
			}
		}
		glFlush();
	};

	bool State_2048::move_down() {
		bool moved_finished;
		bool merged[3][4]{};
		bool rtn = false;
		do {
			moved_finished = true;
			int cur_moves[3][4]{};
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 4; j++) {
					if (Board[i + 1][j]&&
						((!Board[i][j] || ( i != 0 && cur_moves[i - 1][j] ) )) ) {
						cur_moves[i][j] = 1;
						moved_finished = false;
						rtn = true;
					}
					else if (Board[i+1][j] 
							 &&Board[i][j] == Board[i + 1][j] 
							 && ( i == 0 ||(cur_moves[i - 1][j] != 2))
							 && ( !merged[i][j] ) && ( i == 2 || !merged[i + 1][j] )) {
						cur_moves[i][j] = 2;
						if(i) cur_moves[i - 1][j] = 0;
						moved_finished = false;
						rtn = true;
					}
				}
			}
			const auto start_time = std::chrono::high_resolution_clock::now();
			bool animation_finish = false;
			while (!animation_finish) {
				float time_passed =
					std::chrono::duration_cast<std::chrono::duration<float>>
					( std::chrono::high_resolution_clock::now() - start_time ).count();
				const float scale = 10.0f;
				if (time_passed*scale >= 0.5f) {
					animation_finish = true; 
					time_passed = 0.5f / scale;
				}
				for (int i = 0; i < 3; i++) {
					for (int j = 0; j < 4; j++) {
						if (cur_moves[i][j]) clearBoard(j, j, i, i + 1);
					}
				}
				for (int i = 0; i < 3; i++) {
					for (int j = 0; j < 4; j++) {
						switch (cur_moves[i][j]) {
							case 1:
								drawBlock(Board[i + 1][j], j*0.5f - 0.75f, ( i + 1 )*0.5f - 0.75f - time_passed * scale);
								break;
							case 2:
								drawBlock(Board[i + 1][j] + 1, j*0.5f - 0.75f, ( i + 1 )*0.5f - 0.75f - time_passed * scale);
								break;
							case 0:
								break;
						}
					}
				}
				glFlush();
			}
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 4; j++) {
					switch (cur_moves[i][j]) {
						case 1:
							Board[i][j] = Board[i + 1][j];
							Board[i + 1][j] = 0;
							if (i != 2) {
								merged[i][j] = merged[i + 1][j];
							} 
							else {
								merged[i][j] = false;
							}
							break;
						case 2:
							Board[i][j] = Board[i + 1][j] + 1;
							Board[i + 1][j] = 0;
							merged[i][j] = true;
							break;
						case 0:
							break;
					}
				}
			}
		} while (!moved_finished);
		return rtn;
	}
	bool State_2048::move_left() {
		bool moved_finished;
		bool merged[4][3]{};
		bool rtn = false;
		do {
			moved_finished = true;
			int cur_moves[4][3]{};
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 3; j++) {
					if (Board[i][j + 1]&&((!Board[i][j] || ( j != 0 && cur_moves[i][j - 1]) ))) {
						cur_moves[i][j] = 1;
						moved_finished = false;
						rtn = true;
					}
					else if (Board[i][j+1]
							 &&Board[i][j]==Board[i][j+1]
							 &&(j==0||cur_moves[i][j-1]!=2)
							 &&(!merged[i][j])&&(j == 2||!merged[i][j+1])) {
						cur_moves[i][j] = 2;
						if (j) cur_moves[i][j - 1] = 0;
						moved_finished = false;
						rtn = true;
					}
				}
			}
			const auto start_time = std::chrono::high_resolution_clock::now();
			bool animation_finish = false;
			while (!animation_finish) {
				float time_passed =
					std::chrono::duration_cast<std::chrono::duration<float>>
					( std::chrono::high_resolution_clock::now() - start_time ).count();
				const float scale = 10.0f;
				if (time_passed*scale >= 0.5f) {
					animation_finish = true;
					time_passed = 0.5f / scale;
				}
				for (int i = 0; i < 4; i++) {
					for (int j = 2; j >=0; j--) {
						if (cur_moves[i][j]) clearBoard(j, j + 1 , i, i);
					}
				}
				for (int i = 0; i < 4; i++) {
					for (int j = 2; j >= 0; j--) {
						switch (cur_moves[i][j]) {
							case 1:
								drawBlock(Board[i][j + 1], ( j + 1 )*0.5f - 0.75f - time_passed * scale, ( i )*0.5f - 0.75f);
								break;
							case 2:
								drawBlock(Board[i][j + 1] + 1, ( j + 1 )*0.5f - 0.75f - time_passed * scale, ( i )*0.5f - 0.75f);
								break;
							case 0:
								break;
						}
					}
				}
				glFlush();
			}
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 3; j++) {
					switch (cur_moves[i][j]) {
						case 1:
							Board[i][j] = Board[i][j + 1];
							Board[i][j + 1] = 0;
							if (j !=2 ) {
								merged[i][j] = merged[i][j +1];
							}
							else {
								merged[i][j] = false;
							}
							break;
						case 2:
							Board[i][j] = Board[i][j + 1] + 1;
							Board[i][j + 1] = 0;
							merged[i][j] = true;
							break;
						case 0:
							break;
					}
				}
			}
		} while (!moved_finished);
		return rtn;
	}
	bool State_2048::move_right() {
		bool moved_finished;
		bool merged[4][3]{};
		bool rtn = false;
		do {
			moved_finished = true;
			int cur_moves[4][3]{};
			for (int i = 0; i < 4; i++) {
				for (int j = 2; j >=0; j--) {
					if ( Board[i][j]&&((!Board[i][j + 1] || ( j != 2 && cur_moves[i][j + 1]) ))) {
						cur_moves[i][j] = 1;
						moved_finished = false;
						rtn = true;
					}
					else if (Board[i][j]
							 &&Board[i][j]==Board[i][j+1]
							 &&(j==2||cur_moves[i][j+1]!=2)
							 &&(!merged[i][j]&&(j==0||!merged[i][j-1]))) {
						cur_moves[i][j] = 2;
						if (j < 2) cur_moves[i][j + 1] = 0;
						moved_finished = false;
						rtn = true;
					}
				}
			}
			const auto start_time = std::chrono::high_resolution_clock::now();
			bool animation_finish = false;
			while (!animation_finish) {
				float time_passed =
					std::chrono::duration_cast<std::chrono::duration<float>>
					( std::chrono::high_resolution_clock::now() - start_time ).count();
				const float scale = 10.0f;
				if (time_passed*scale >= 0.5f) {
					animation_finish = true;
					time_passed = 0.5f / scale;
				}
				for (int i = 0; i < 4; i++) {
					for (int j = 2; j >=0; j--) {
						if (cur_moves[i][j]) clearBoard(j, j + 1, i, i);
					}
				}
				for (int i = 0; i < 4; i++) {
					for (int j = 2; j >= 0; j--) {
						switch (cur_moves[i][j]) {
							case 1:
								drawBlock(Board[i][j], j*0.5f - 0.75f + time_passed * scale, i*0.5f - 0.75f);
								break;
							case 2:
								drawBlock(Board[i][j] + 1, j*0.5f - 0.75f + time_passed * scale, i*0.5f - 0.75f);
								break;
							case 0:
								break;
						}
					}
				}
				glFlush();
			}
			for (int i = 0; i < 4; i++) {
				for (int j = 2; j >= 0; j--) {
					switch (cur_moves[i][j]) {
						case 1:
							Board[i][j + 1] = Board[i][j];
							Board[i][j] = 0;
							if (j != 0) {
								merged[i][j] = merged[i][j - 1];
							}
							else {
								merged[i][j] = false;
							}
							break;
						case 2:
							Board[i][j + 1] = Board[i][j] + 1;
							Board[i][j] = 0;
							merged[i][j] = true;
							break;
						case 0:
							break;
					}
				}
			}
		} while (!moved_finished);
		return rtn;
	}
	bool State_2048::move_up() {
		bool moved_finished;
		bool merged[3][4]{};
		bool rtn = false;
		do {
			moved_finished = true;
			int cur_moves[3][4]{};
			for (int i = 2; i >=0; i--) {
				for (int j = 0; j < 4; j++) {
					if (Board[i][j]&&((!Board[i+1][j] || ( i != 2 && cur_moves[i + 1][j]) ) )) {
						cur_moves[i][j] = 1;
						moved_finished = false;
						rtn = true;
					} else if (Board[i][j]
							   && Board[i + 1][j] == Board[i][j]
							   && ( i == 2 || ( cur_moves[i + 1][j] != 2 ))
							   && (( !merged[i][j] ) && ( i == 0 || !merged[i-1][j] ))) {
							cur_moves[i][j] = 2;
							if (i < 2) cur_moves[i + 1][j] = 0;
							moved_finished = false;
							rtn = true;
					}
				}
			}
			const auto start_time = std::chrono::high_resolution_clock::now();
			bool animation_finish = false;
			while (!animation_finish) {
				float time_passed =
					std::chrono::duration_cast<std::chrono::duration<float>>
					( std::chrono::high_resolution_clock::now() - start_time ).count();
				const float scale = 10.0f;
				if (time_passed*scale >= 0.5f) {
					animation_finish = true;
					time_passed = 0.5f / scale;
				}
				for (int i = 2; i >=0; i--) {
					for (int j = 0; j < 4; j++) {
						if (cur_moves[i][j]) clearBoard(j, j, i, i + 1);
					}
				}
				for (int i = 2; i >= 0; i--) {
					for (int j = 0; j < 4; j++) {
						switch (cur_moves[i][j]) {
							case 1:
								drawBlock(Board[i][j], j*0.5f - 0.75f, i*0.5f - 0.75f + time_passed * scale);
								break;
							case 2:
								drawBlock(Board[i][j] + 1, j*0.5f - 0.75f, i*0.5f - 0.75f + time_passed * scale);
								break;
							case 0:
								break;
						}
					}
				}
				glFlush();
			}
			for (int i = 2; i >=0; i--) {
				for (int j = 0; j < 4; j++) {
					switch (cur_moves[i][j]) {
						case 1:
							Board[i + 1][j] = Board[i][j];
							Board[i][j] = 0;
							if (i != 0) {
								merged[i][j] = merged[i -1][j];
								merged[i - 1][j] = false;
							}
							else {
								merged[i][j] = false;
							}
							break;
						case 2:
							Board[i + 1][j] = Board[i][j] + 1;
							Board[i][j] = 0;
							merged[i][j] = true;					
							break;
						case 0:
							break;
					}
				}
			}
		} while (!moved_finished);
		return rtn;
	}

	void State_2048::clearBoard(int x0, int x1, int y0, int y1) {
		TextureModel->data.pos = glm::vec4(x0 * 0.5f - 1.0f, y0 * 0.5f - 1.0f, 0.0f, 1.0f);
		TextureModel->data.vec1 = glm::vec3(( x1 - x0 + 1 ) * 0.5f, 0.0f, 0.0f);
		TextureModel->data.vec2 = glm::vec3(0.0f, ( y1 - y0 + 1 ) * 0.5f, 0.0f);
		TextureModel->data.tex_ori = glm::vec2(x0 / 8.0f, y0 / 4.0f);
		TextureModel->data.tex_vec1 = glm::vec2(( x1 - x0 + 1 ) / 8.0f, 0.0f);
		TextureModel->data.tex_vec2 = glm::vec2(0.0f, ( y1 - y0 + 1 ) / 4.0f);
		TextureModel->Update();
		TextureModel->Draw();
	}
	void State_2048::drawBlock(unsigned char block, float Xc, float Yc) {
		TextureModel->data.pos = glm::vec4(Xc - 0.25f, Yc - 0.25f, 0.0f, 1.0f);
		TextureModel->data.vec1 = glm::vec3(0.5f, 0.0f, 0.0f);
		TextureModel->data.vec2 = glm::vec3(0.0f, 0.5f, 0.0f);
		TextureModel->data.tex_ori =
			glm::vec2(0.5f + ( ( block - 1 ) % 4 ) / 8.0f, ( ( block - 1 ) / 4 ) / 4.0f);
		TextureModel->data.tex_vec1 = glm::vec2(0.125f, 0.0f);
		TextureModel->data.tex_vec2 = glm::vec2(0.0f, 0.25f);
		TextureModel->Update();
		TextureModel->Draw();
	}

	void State_2048::Refresh(size_t ID) {
		if(!quit_key)
			TextureModel->Begin();
		TextureModel->data.tex_ori = glm::vec2(0.0f, 0.0f);
		TextureModel->data.tex_vec1 = glm::vec2(0.5f, 0.0f);
		TextureModel->data.tex_vec2 = glm::vec2(0.0f, 1.0f);
		TextureModel->data.pos = glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f);
		TextureModel->data.vec1 = glm::vec3(2.0f, 0.0f, 0.0f);
		TextureModel->data.vec2 = glm::vec3(0.0f, 2.0f, 0.0f);
		TextureModel->Update();
		TextureModel->Draw();
		TextureModel->data.vec1 = glm::vec3(0.5f, 0.0f, 0.0f);
		TextureModel->data.vec2 = glm::vec3(0.0f, 0.5f, 0.0f);
		TextureModel->data.tex_vec1 = glm::vec2(1.0f / 8.0f, 0.0f);
		TextureModel->data.tex_vec2 = glm::vec2(0.0f, 1.0f / 4.0f);
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				if (Board[i][j]) {
					TextureModel->data.pos = glm::vec4(j*0.5f - 1.0f, i*0.5f - 1.0f, 0.0f, 1.0f);
					TextureModel->data.tex_ori = glm::vec2(0.5f + 0.125f*( ( Board[i][j] - 1 ) % 4 ), 0.25f*( ( Board[i][j] - 1 ) / 4 ));
					TextureModel->Update();
					TextureModel->Draw();
				}
			}
		}
		if (!quit_key) {
			renderExitText();
		}
		glFlush();
	}

	void State_2048::renderExitText() {
		BlockModel->data.Block_center = glm::vec2(0.0f, 0.0f);
		BlockModel->data.Block_size = glm::vec2(0.8f, 0.25f);
		BlockModel->data.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		BlockModel->data.radius = 0.2f;
		BlockModel->Begin();
		BlockModel->Update();
		BlockModel->Draw();
		BlockModel->data.Block_size = glm::vec2(0.78f, 0.23f);
		BlockModel->data.color = glm::vec4(0xF5 / 255.0f, 0xF6 / 255.0f, 0xCE / 255.0f, 1.0f);
		BlockModel->Update();
		BlockModel->Draw();
		size_t width, height;
		Managers.ContextManager.GetCurrentDefaultResolution(width, height);
		const char* message1 = "Game Over!";
		const float scale1 = 0.8f;
		const char* message2 = "Press 'Q' Key to exit!";
		const float scale2 = 0.4f;
		const float divide = 32.0f;
		float text_length1 = Managers.TextManager.TextLength(message1);
		float text_length2 = Managers.TextManager.TextLength(message2);
		float text_height = Managers.TextManager.GetHeight();
		float x1 = width / 2.0f - text_length1 * scale1 / 2.0f;
		float x2 = width / 2.0f - text_length2 * scale2 / 2.0f;
		float y2 = height / 2.0f - ( text_height * ( scale1 + scale2 ) + divide ) / 2.0f *0.8f;
		float y1 = y2 + text_height * (scale2)+divide / 2;
		Managers.TextManager.renderText(message1, glm::vec2(x1, y1), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), scale1);
		Managers.TextManager.renderText(message2, glm::vec2(x2, y2), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), scale2);
	}
}



		/*
		int moves[3][4]{};
		for (int j = 0; j < 4; j++) {
			int i = 0;
			int k = 0;
			while (k < 4) {
				int a = k;
				while (k < 4 && !Board[k][j]) k++;
				if (k == 4) break;
				int b = k;
				while (k < 4 && Board[k][j]) k++;
				if (k == 4) break;
				if (k) {
					int blank = b - a;
					int blocks = k - b;
					for (int c = 0; c < blocks; c++) {
						moves[a + c][j] = blank;
					}
				}
			}
		}
		*/
