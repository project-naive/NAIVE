#pragma once

#include <Graphics\Shaders\Shader.h>

namespace State_2048 {
	namespace Shaders {
		class TextureQuad: public Engine::Graphics::Shader {
		public:
			TextureQuad(Engine::Graphics::Managers::Shader& given_manager): Shader(given_manager) {
				CreateProgram();
			}
			GLuint CreateProgram() override;
			const char* fragment_shader_path = ".\\Samples\\2048\\2048\\States\\Simple\\Models\\TextureQuad\\TextureQuad.frag";
			const char* vertex_shader_path = ".\\Samples\\2048\\2048\\States\\Simple\\Models\\TextureQuad\\TextureQuad.vert";
		};
	}
}
