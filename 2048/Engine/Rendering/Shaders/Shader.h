/*****************************************************************
*	This section defines a class that manages shader programs.
*	It is also responsible for calling the OpenGL APIs when needed.
*
*
*/

#pragma once

#include <GL\glew.h>

namespace Rendering {
	class Shader {
	public:
		Shader() = default;
		GLuint program = 0;
		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;
		Shader(Shader&& src) {
			program = src.program;
			src.program = 0;
		}
		Shader& operator=(Shader&& src) {
			program = src.program;
			src.program = 0;
			return *this;
		}
		~Shader() {
			if (program) {
				glDeleteProgram(program);
			}
		}
		virtual GLuint CreateProgram() { return 0; }
		GLuint CreateShader(GLenum shaderType, const char* shadercode, GLint length = 0) const;
	};
}
