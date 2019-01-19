#include "Text.h"

#include "..\..\Managers\Random\ShaderManager.h"

namespace Engine{
	namespace Graphics{
		namespace Models{
			Text::Text(Managers::Shader& given_manager):
				Model(given_manager, Managers::Shader::Text){
				vbos = new GLuint[1];
				vbo_count = 1;
				glGenVertexArrays(1, &vao);
				glBindVertexArray(vao);
				glGenBuffers(1, vbos);
				glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(VertexFormat)*6, nullptr, GL_DYNAMIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
			}
			void Text::Update() {
				if(!VetexData) return;
				glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(VertexFormat)*6, VetexData);
			}
			void Text::Draw() {
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
			GLuint Text::Begin(){
				glBindVertexArray(vao);
				return manager.UseDefault(Managers::Shader::Text);
			}
		}
	}
}


