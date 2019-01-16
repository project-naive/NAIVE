#include "Triangle.h"

#include "..\..\Managers\Random\ShaderManager.h"
#include "..\..\..\Core\Game.h"


namespace Engine {
	namespace Graphics {
		namespace Models {
			Triangle::Triangle(Managers::Shader& given_manager): Model(given_manager) {
				shader = Managers::Shader::Naive;
				vbos = new GLuint[1]{};
				vbo_count = 1;
				glGenVertexArrays(1, &vao);
				glBindVertexArray(vao);
				glGenBuffers(1, vbos);
				glBindBuffer(GL_ARRAY_BUFFER,vbos[0]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(VertexFormat) * 3, nullptr, GL_DYNAMIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(
	                0,
					3,
					GL_FLOAT,
					GL_FALSE,
					sizeof(VertexFormat),
					(void*)0);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(
					1,
					4,
					GL_FLOAT,
					GL_FALSE,
					sizeof(VertexFormat),
					(void*)offsetof(VertexFormat, VertexFormat::color)
				);
			}

			void Triangle::Update() {
				glBindBuffer(GL_ARRAY_BUFFER,vbos[0]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(VertexFormat) * 3, VertexData);
			}

			void Triangle::Draw() {
				glDrawArrays(GL_TRIANGLES,0,3);
			}

			GLuint Triangle::Begin() {
				glBindVertexArray(vao);
				return manager.UseDefault(Managers::Shader::Naive);;;
			}
		}
	}
}
