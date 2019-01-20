#include "TextureQuad.h"

#include <Graphics\Managers\Random\ShaderManager.h>
#include <glm\gtc\type_ptr.hpp>

namespace State_2048 {
	namespace Models {
		TextureQuad::TextureQuad(Engine::Graphics::Managers::Shader& given_manager, size_t shaderID):
			Model(given_manager, shaderID) {
			manager.UseGeneric(shaderID);
			projection_uniform_location = 
				glGetUniformLocation(manager.GetCurrent(), "projection");
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
			vbos = new GLuint[1];
			glGenBuffers(1, vbos);
			vbo_count = 1;
			glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(VertexFormat) * 6, nullptr, GL_DYNAMIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 
								  sizeof(VertexFormat), (void*)offsetof(VertexFormat, VertexFormat::tex_coords));
		}

		GLuint TextureQuad::Begin() {
			glBindVertexArray(vao);
			glBindTexture(GL_TEXTURE_2D, data.textureID);
			return manager.UseGeneric(shader);
		};

		void TextureQuad::Draw() {
			glUniformMatrix4fv(projection_uniform_location, 1, GL_FALSE, glm::value_ptr(data.projection));
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		void TextureQuad::Update() {
			vertices[0].position = data.pos;
			vertices[0].tex_coords = data.tex_ori;
			vertices[1].position = data.pos + glm::vec4(data.vec1, 0.0f);
			vertices[1].tex_coords = data.tex_ori + data.tex_vec1;
			vertices[2].position = data.pos + glm::vec4(data.vec1 + data.vec2, 0.0f);
			vertices[2].tex_coords = data.tex_ori + data.tex_vec1 + data.tex_vec2;
			vertices[3].position = data.pos + glm::vec4(data.vec1 + data.vec2, 0.0f);
			vertices[3].tex_coords = data.tex_ori + data.tex_vec1 + data.tex_vec2;
			vertices[4].position = data.pos + glm::vec4(data.vec2, 0.0f);
			vertices[4].tex_coords = data.tex_ori + data.tex_vec2;
			vertices[5].position = data.pos;
			vertices[5].tex_coords = data.tex_ori;
			glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(VertexFormat) * 6, vertices);
		}
	}
}
