#pragma once

#include <Graphics\Shaders\Shader.h>

namespace State_2048 {
	namespace Shaders {
		class Block: public Engine::Graphics::Shader {
		public:
			Block(Engine::Graphics::Managers::Shader& given_manager):
				Shader(given_manager) {
				CreateProgram();
			}
			GLuint CreateProgram() override final;
			const char* fragment_shader_path = ".\\2048\\2048\\States\\State_2048\\Models\\Block\\Block.frag";
			const char* vertex_shader_path = ".\\2048\\2048\\States\\State_2048\\Models\\Block\\Block.vert";
		};
	}
}