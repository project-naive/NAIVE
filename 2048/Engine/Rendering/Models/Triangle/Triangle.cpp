#include "Triangle.h"

#include "..\..\Managers\ShaderManager.h"
#include "..\..\Managers\VertexAttributeManager.h"

namespace Rendering {
	namespace Models {
		Triangle::Triangle() {
			shader = Manager::Shader::GetProgram(Manager::Shader::Naive);
			vbos = new GLuint[1]{};
			vbo_count = 1;
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
			glGenBuffers(1, vbos);
			glBindBuffer(vbos[0], GL_ARRAY_BUFFER);
			glBufferData(vbos[0], sizeof(VertexFormat) * sizeof(VertexData), nullptr, GL_DYNAMIC_DRAW);
			position_array_attribute = Manager::VertexAttribute::RequestAttribute();
			if (position_array_attribute == GLuint(-1)) {
				glDeleteVertexArrays(1, &vao);
				glDeleteBuffers(1, vbos);
				delete[] vbos;
				vao = 0;
				vbos = nullptr;
				vbo_count = 0;
				return;
			}
			glVertexAttribPointer(
				position_array_attribute,
				sizeof(VertexData),
				GL_FLOAT,
				GL_FALSE,
				sizeof(VertexFormat),
				(void*)0);
			color_array_attribute = Manager::VertexAttribute::RequestAttribute();
			if (color_array_attribute == GLuint(-1)) {
				glDeleteVertexArrays(1, &vao);
				glDeleteBuffers(1, vbos);
				delete[] vbos;
				vao = 0;
				vbos = nullptr;
				vbo_count = 0;
				return;
			}
			glVertexAttribPointer(
				color_array_attribute,
				sizeof(VertexData),
				GL_FLOAT,
				GL_FALSE,
				sizeof(VertexFormat),
				(void*)offsetof(VertexFormat, VertexFormat::color)
			);
			glBindVertexArray(0);
		}

		void Triangle::Draw() {
			glUseProgram(shader);
			glBindVertexArray(vao);
			glBufferSubData(vbos[0], 0, sizeof(VertexFormat) * sizeof(VertexData), VertexData);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
}

