#include "Fan.h"

#include <glm\gtc\type_ptr.hpp>

namespace State_2048{
	namespace Models{
		Fan::Fan(Engine::Graphics::Managers::Shader& given_manager, size_t shaderID):
			Model(given_manager, shaderID) {
			given_manager.UseGeneric(shaderID);
			projection_uniform_location = glGetUniformLocation(manager.GetCurrent(), "projection");
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
			vbos = new GLuint[1];
			glGenBuffers(1, vbos);
			vbo_count = 1;
			glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(VertexFormat)*6, nullptr, GL_DYNAMIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)offsetof(VertexFormat, VertexFormat::color));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)offsetof(VertexFormat, VertexFormat::uv_coords));
		}
		GLuint State_2048::Models::Fan::Begin(){
			GLuint program = manager.UseGeneric(shader);
			glUniformMatrix4fv(projection_uniform_location, 1, GL_FALSE, glm::value_ptr(projection));
			glBindVertexArray(vao);
			return program;
		}

		void State_2048::Models::Fan::Draw() {
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		void State_2048::Models::Fan::Update(){
			glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
			glm::vec3 vec1n = glm::normalize(data.vec1);
			glm::vec3 vec2n = glm::normalize(data.vec2);
			VertexData[0] = {
				data.origin,
				data.origin_color,
				glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
			};
			VertexData[1] = {
				data.origin + glm::vec4(data.vec1, 0.0f),
				data.color1,
				glm::vec4(vec1n, 1.0f)
			};
			VertexData[2] = {
				data.origin + glm::vec4(data.vec1+data.vec2, 0.0f),
				data.far_color,
				glm::vec4(vec1n + vec2n, 1.0f)
			};
			VertexData[3] = {
				data.origin + glm::vec4(data.vec1+data.vec2, 0.0f),
				data.far_color,
				glm::vec4(vec1n + vec2n, 1.0f)
			};
			VertexData[4] = {
				data.origin + glm::vec4(data.vec2, 0.0f),
				data.color2,
				glm::vec4(vec2n, 1.0f)
			};
			VertexData[5] = {
				data.origin,
				data.origin_color,
				glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
			};
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(VertexFormat)*6, VertexData);
		}
	}
}





