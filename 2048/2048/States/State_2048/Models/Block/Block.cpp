#include "Block.h"
#include <glm\gtc\type_ptr.hpp>
#include "..\..\State_2048.h"

namespace State_2048 {
	namespace Models {
		Block::Block(Engine::Graphics::Managers::Shader& given_manager, size_t shaderID):
			Model(given_manager, shaderID) {
			GLuint program = given_manager.UseGeneric(shaderID);
			projection_uniform_location =
				glGetUniformLocation(program, "projection");
			color_uniform_location =
				glGetUniformLocation(program, "color");
			radius_uniform_location =
				glGetUniformLocation(program, "radius");
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
			vbos = new GLuint[1];
			vbo_count = 1;
			glGenBuffers(1, vbos);
			glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(VertexFormat) * 6, nullptr, GL_DYNAMIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
		};

		GLuint Block::Begin() {
			glBindVertexArray(vao);
			return manager.UseGeneric(shader);
		}
		void Block::Draw() {
			glUniformMatrix4fv(projection_uniform_location, 1, GL_FALSE, glm::value_ptr(data.projection));
			glUniform1f(radius_uniform_location, data.radius);
			glUniform4f(color_uniform_location,
						data.color.r, data.color.g, data.color.b, data.color.a);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		void Block::Update() {
			float x1, x2, y1, y2;
			x1 = data.Block_center.x - data.Block_size.x;
			x2 = data.Block_center.x + data.Block_size.x;
			y1 = data.Block_center.y - data.Block_size.y;
			y2 = data.Block_center.y + data.Block_size.y;
			vertices[0].x = x1;
			vertices[0].y = y2;
			vertices[1].x = x2;
			vertices[1].y = y2;
			vertices[2].x = x2;
			vertices[2].y = y1;
			vertices[3].x = x1;
			vertices[3].y = y2;
			vertices[4].x = x1;
			vertices[4].y = y1;
			vertices[5].x = x2;
			vertices[5].y = y1;
			glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(VertexFormat) * 6, vertices);
		}
	}
}


