#pragma once

#include "..\Shader.h"

namespace Engine{
	namespace Graphics{
		namespace Shaders{
			class Text: public Shader {
			public:
				Text() {
					CreateProgram();
				}
				GLuint CreateProgram() override;
				const char* fragment_shader_path = ".\\Engine\\Graphics\\Shaders\\Text\\Text.frag";
				const char* vertex_shader_path = ".\\Engine\\Graphics\\Shaders\\Text\\Text.vert";
			};
		}
	}
}
