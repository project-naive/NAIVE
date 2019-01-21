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
		BlockModel->data.Block_size = glm::vec2(1.0f, 1.0f);
		BlockModel->data.color = glm::vec4(0xC6 / 255.0f, 0xC3 / 255.0f, 0xBA / 255.0f, 1.0f);
		BlockModel->data.radius = 0.0f;
		BlockModel->data.projection = glm::identity<glm::mat4>();
		BlockModel->Begin();
		glViewport(0, 0, width, height);
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
//		TextureModel->Begin();
//		TextureModel->Draw();
		float time = std::chrono::duration_cast<std::chrono::duration<float> > ( std::chrono::steady_clock::now() - start_time ).count();
//		clearBoard(0, 3, 0, 3);
//		drawBlock(10, sinf(time), cosf(time));
		while (true) {
			bool okay = false;
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
			int move = std::rand() % 4;
			switch (move) {
				case 0:
					move_down();
					break;
				case 1:
					move_left();
					break;
				case 2:
					move_right();
					break;
				case 3:
					move_up();
					break;
			}
		}
		move_right();
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
		/*
		char input = rand() % 4;
		switch (input) {
			case 0:
				move_down();
				break;
			case 1:
				move_left();
				break;
			case 2:
				move_right();
				break;
			case 3:
				move_up();
				break;
		}
		*/

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
					if (!Board[i][j] && Board[i+1][j]) {
						cur_moves[i][j] = 1;
						moved_finished = false;
						rtn = true;
					}
					else if (Board[i+1][j] 
							 &&Board[i][j] == Board[i + 1][j] 
							 && ( i == 0 ||(cur_moves[i - 1][j] != 2))
							 && ( !merged[i][j] ) && ( i == 2 || !merged[i + 1][j] )) {
						cur_moves[i][j] = 2;
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
						switch (cur_moves[i][j]) {
							case 1:
								clearBoard(j, j, i, i + 1);
								drawBlock(Board[i + 1][j], j*0.5f - 0.75f, ( i + 1 )*0.5f - 0.75f - time_passed * scale);
								break;
							case 2:
								clearBoard(j, j, i, i + 1);
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
							if (i < 2) {
								merged[i][j] = merged[i + 1][j];
								merged[i + 1][j] = false;
							} else {
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
					if (!Board[i][j] && Board[i][j + 1]) {
						cur_moves[i][j] = 1;
						moved_finished = false;
						rtn = true;
					}
					else if (Board[i][j+1]
							 &&Board[i][j]==Board[i][j+1]
							 &&(j==0||cur_moves[i][j-1]!=2)
							 &&(!merged[i][j])&&(i == 2||!merged[i][j+1])) {
						cur_moves[i][j] = 2;
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
					for (int j = 0; j <3; j++) {
						switch (cur_moves[i][j]) {
							case 1:
								clearBoard(j, j + 1 , i, i);
								drawBlock(Board[i][j + 1], (j+1)*0.5f - 0.75f - time_passed * scale, (i)*0.5f - 0.75f );
								break;
							case 2:
								clearBoard(j, j + 1, i, i);
								drawBlock(Board[i][j + 1] + 1, (j+1)*0.5f - 0.75f - time_passed * scale, (i)*0.5f - 0.75f);
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
							if (i < 2) {
								merged[i][j] = merged[i][j + 1];
								merged[i][j + 1] = false;
							} else {
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
				for (int j = 2; j >= 0; j--) {
					if (!Board[i][j + 1] && Board[i][j]) {
						cur_moves[i][j] = 1;
						moved_finished = false;
						rtn = true;
					}
					else if (Board[i][j]
							 &&Board[i][j]==Board[i][j+1]
							 &&(j==2||cur_moves[i][j+1]!=2)
							 &&(!merged[i][j]&&(j==0||!merged[i][j-1]))) {
						cur_moves[i][j] = 2;
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
						switch (cur_moves[i][j]) {
							case 1:
								clearBoard(j, j + 1, i, i);
								drawBlock(Board[i][j], j*0.5f - 0.75f + time_passed * scale, i*0.5f - 0.75f);
								break;
							case 2:
								clearBoard(j, j + 1, i, i);
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
							merged[i][j] = false;
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
					if (!Board[i+1][j] && Board[i][j]) {
						cur_moves[i][j] = 1;
						moved_finished = false;
						rtn = true;
					} else if (Board[i][j]
							   && Board[i + 1][j] == Board[i][j]
							   && ( i == 2 || ( cur_moves[i + 1][j] != 2 ))
							   && (( !merged[i][j] ) && ( i == 0 || !merged[i-1][j] ))) {
							cur_moves[i][j] = 2;
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
						switch (cur_moves[i][j]) {
							case 1:
								clearBoard(j, j, i, i + 1);
								drawBlock(Board[i][j], j*0.5f - 0.75f, i*0.5f - 0.75f + time_passed * scale);
								break;
							case 2:
								clearBoard(j, j, i, i + 1);
								drawBlock(Board[i][j] + 1, j*0.5f - 0.75f, i*0.5f - 0.75f + time_passed * scale);
								break;
							case 0:
								break;
						}
					}
				}
				glFlush();
			}
			for (int i = 2; i >= 0; i--) {
				for (int j = 0; j < 4; j++) {
					switch (cur_moves[i][j]) {
						case 1:
							Board[i + 1][j] = Board[i][j];
							Board[i][j] = 0;
							if (i != 0) {
								merged[i][j] = merged[i - 1][j];
							}
							merged[i][j] = false;
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
		float texX0 = x0 / 8.0f ;
		float texX1 = ( x1 - x0 + 1 ) / 8.0f;
		float texY0 = y0 / 4.0f;
		float texY1 = ( y1 - y0 + 1 ) / 4.0f;
		float X0 = x0 * 0.5f - 1.0f;
		float Y0 = y0 * 0.5f - 1.0f;
		float X1 = ( x1 - x0 + 1) * 0.5f;
		float Y1 = ( y1 - y0 + 1) * 0.5f;
		TextureModel->data.pos = glm::vec4(X0, Y0, 0.0f, 1.0f);
		TextureModel->data.vec1 = glm::vec3(X1, 0.0f, 0.0f);
		TextureModel->data.vec2 = glm::vec3(0.0f, Y1, 0.0f);
		TextureModel->data.tex_ori = glm::vec2(texX0, texY0);
		TextureModel->data.tex_vec1 = glm::vec2(texX1, 0.0f);
		TextureModel->data.tex_vec2 = glm::vec2(0.0f, texY1);
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
