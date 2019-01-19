/*****************************************************************
*	This section defines a class that manages shader programs.
*	It is also responsible for calling the OpenGL APIs when needed.
*
*
*/

#pragma once

#include <GL\glew.h>

namespace Engine {
	namespace Graphics {
		namespace Managers{
			class Shader;
		}
		class Shader {
		public:
			Shader(Managers::Shader& given_manager):parent(given_manager) {}
			Managers::Shader& parent;
			GLuint program = 0;
			Shader(const Shader&) = delete;
			Shader(Shader&& src) = delete;
			Shader& operator=(const Shader&) = delete;
			Shader& operator=(Shader&& src) = delete;
			~Shader() {
				if (program) {
					glDeleteProgram(program);
				}
			}
			virtual GLuint CreateProgram() { return 0; }
			GLuint CreateShader(GLenum shaderType, const char* shadercode, GLint length = 0) const;
		};
	}
}
