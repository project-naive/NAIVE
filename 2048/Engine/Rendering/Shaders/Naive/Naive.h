#pragma once

#include "..\Shader.h"

namespace Rendering {
	namespace Shaders {
		class Naive: public Shader {
		public:
			Naive() {
				CreateProgram();
			}
			GLuint CreateProgram() override;
			const char* fragment_shader_path = ".\\Engine\\Rendering\\Shaders\\Naive\\Naive.frag";
			const char* vertex_shader_path = ".\\Engine\\Rendering\\Shaders\\Naive\\Naive.vert";
		};
	}
}

