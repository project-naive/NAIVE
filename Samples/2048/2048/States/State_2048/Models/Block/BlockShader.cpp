#include "BlockShader.h"

#include <iostream>
#include <fstream>
#include <vector>

namespace State_2048 {
	namespace Shaders {
		GLuint Block::CreateProgram() {
			std::ifstream ifs;
			ifs.open(vertex_shader_path);
			if (!ifs) {
				std::cerr << "Error opening Block vertex shader file!\n" << std::endl;
				return 0;
			}
			ifs.seekg(0, std::ios::end);
			GLint size = GLint(ifs.tellg());
			std::vector<char> code(size);
			ifs.seekg(0);
			ifs.read(&code[0], size);
			ifs.close();
			GLuint VertexShader = CreateShader(GL_VERTEX_SHADER, &code[0]);
			if (!VertexShader) return 0;
			code.clear();
			ifs.open(fragment_shader_path);
			if (!ifs) {
				std::cerr << "Error opening Block fragment shader file!\n" << std::endl;
				glDeleteShader(VertexShader);
				return 0;
			}
			ifs.seekg(0, std::ios::end);
			size = GLint(ifs.tellg());
			code.resize(size);
			ifs.seekg(0);
			ifs.read(&code[0], size);
			ifs.close();
			GLuint FragmentShader = CreateShader(GL_FRAGMENT_SHADER, &code[0], size);
			if (!FragmentShader) {
				glDeleteShader(VertexShader);
				return 0;
			}
			const GLuint created_program = glCreateProgram();
			glAttachShader(created_program, VertexShader);
			glAttachShader(created_program, FragmentShader);
			glLinkProgram(created_program);
			GLint link_result;
			glGetProgramiv(created_program, GL_LINK_STATUS, &link_result);
			if (link_result == GL_FALSE) {
				GLint info_log_length;
				glGetProgramiv(created_program, GL_INFO_LOG_LENGTH, &info_log_length);
				std::vector<char> info_log(info_log_length);
				glGetProgramInfoLog(created_program, info_log_length, nullptr, &info_log[0]);
				std::cerr << "Error linking Block Shader Program!\n\n" << info_log.data() << '\n' << std::endl;
				glDeleteShader(VertexShader);
				glDeleteShader(FragmentShader);
				glDeleteProgram(created_program);
				return 0;
			}
			program = created_program;
			return created_program;
		}
	}
}