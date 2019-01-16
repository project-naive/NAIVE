#include "Fan.h"

#include <glm\gtc\type_ptr.hpp>

namespace State_2048{
	namespace Models{
		Fan::Fan(Engine::Graphics::Managers::Shader& given_manager):Model(given_manager){
			Shaders::Fan* FanShader = new Shaders::Fan(given_manager);
			FanShader_ID = given_manager.addGeneric(*FanShader);
			given_manager.UseGeneric(FanShader_ID);
			projection_uniform_location = glGetUniformLocation(FanShader->program,"projection");
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
			manager.UseGeneric(FanShader_ID);
			glBindVertexArray(vao);
			return -1;
		}

		void State_2048::Models::Fan::Draw() {
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		void State_2048::Models::Fan::Update(){
			glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
			glm::vec3 vec1n = glm::normalize(data.vec1);
			glm::vec3 vec2n = glm::normalize(data.vec2);
			VertexData[0] = {
				glm::vec4(data.origin, 1.0f),
				data.origin_color,
				glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
			};
			VertexData[1] = {
				glm::vec4(data.origin+data.vec1, 1.0f),
				data.color1,
				glm::vec4(vec1n, 1.0f)
			};
			VertexData[2] = {
				glm::vec4(data.origin+data.vec1+data.vec2, 1.0f),
				data.far_color,
				glm::vec4(vec1n + vec2n, 1.0f)
			};
			VertexData[3] = {
				glm::vec4(data.origin+data.vec1+data.vec2, 1.0f),
				data.far_color,
				glm::vec4(vec1n + vec2n, 1.0f)
			};
			VertexData[4] = {
				glm::vec4(data.origin+data.vec2, 1.0f),
				data.color2,
				glm::vec4(vec2n, 1.0f)
			};
			VertexData[5] = {
				glm::vec4(data.origin, 1.0f),
				data.origin_color,
				glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
			};
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(VertexFormat)*6, VertexData);
			glUniformMatrix4fv(projection_uniform_location, 1, GL_FALSE, glm::value_ptr(projection));
		}
	}
}





