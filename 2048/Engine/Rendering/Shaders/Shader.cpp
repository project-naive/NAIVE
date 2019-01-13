#include "Shader.h"

#include <vector>
#include <iostream>

namespace Engine {
	namespace Rendering {
		GLuint Shader::CreateShader(GLenum shaderType, const char* shadercode, GLint length) const {
			const GLuint shader = glCreateShader(shaderType);
			if (!shader) {
				std::cerr << "Error creating shader of type ID " << shaderType << '\n' << std::endl;
				return 0;
			}
			if (length) {
				glShaderSource(shader, 1, &shadercode, &length);
			} else {
				glShaderSource(shader, 1, &shadercode, nullptr);
			}
			glCompileShader(shader);
			GLint compile_result;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_result);
			if (compile_result == GL_FALSE) {
				GLint info_log_length = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
				GLchar* info_log = new GLchar[info_log_length + 1];
				glGetShaderInfoLog(shader, info_log_length, nullptr, info_log);
				std::cerr << "Error compiling shader of type ID " << shaderType << " !\n\n" << info_log << '\n' << std::endl;
				glDeleteShader(shader);
				return 0;
			}
			return shader;
		}
	}
}

