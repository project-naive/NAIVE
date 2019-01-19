#include "TextureCube.h"
#include <Graphics\Managers\Random\ShaderManager.h>
#include <glm\gtc\type_ptr.hpp>

namespace State_2048 {
	namespace Models {
		TextureCube::TextureCube(Engine::Graphics::Managers::Shader& given_manager, size_t shaderID):
			Model(given_manager,shaderID) {
			given_manager.UseGeneric(shaderID);
			projection_uniform_location =
				glGetUniformLocation(manager.GetCurrent(), "projection");
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
			vbos = new GLuint[1];
			glGenBuffers(1, vbos);
			vbo_count = 1;
			glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(VertexFormat)*6*6, nullptr, GL_DYNAMIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 
								  sizeof(VertexFormat), (void*)offsetof(VertexFormat, VertexFormat::tex_coords));
		}
		GLuint TextureCube::Begin() {
			GLuint program = manager.UseGeneric(shader);
			glBindVertexArray(vao);
			glBindTexture(GL_TEXTURE_2D, data.textureID);
			return program;
		}
		void TextureCube::Draw() {
			glUniformMatrix4fv(projection_uniform_location, 1, GL_FALSE, glm::value_ptr(data.projection));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		void TextureCube::Update() {
			vertices[0][0] = VertexFormat{ 
				data.pos,
				glm::vec2(float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Back_x))),
				          float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Back_y))))
			};
			vertices[0][1] = VertexFormat{
	            data.pos + glm::vec4(data.vec2, 0.0f),
	            glm::vec2(float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Back_x))),
			              float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Back_y))))
			};
			vertices[0][2] = VertexFormat{
	            data.pos + glm::vec4(data.vec2 + data.vec1, 0.0f),
	            glm::vec2(float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Back_x))),
			              float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Back_y))))
			};
			vertices[0][3] = VertexFormat{
	            (data.pos + glm::vec4(data.vec2 + data.vec1, 0.0f)),
	            glm::vec2(float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Back_x))),
			              float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Back_y))))
			};
			vertices[0][4] = VertexFormat{
	            data.pos + glm::vec4(data.vec1, 0.0f),
	            glm::vec2(float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Back_x))),
			              float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Back_y))))
			};
			vertices[0][5] = VertexFormat{
	            data.pos,
			    glm::vec2(float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Back_x))),
			              float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Back_y))))
			};
			vertices[1][0] = VertexFormat{
	            data.pos,
	            glm::vec2(float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Bottom_x))),
			              float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Bottom_y))))
			};
			vertices[1][1] = VertexFormat{
				data.pos + glm::vec4(data.vec1, 0.0f),
				glm::vec2(float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Bottom_x))),
						  float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Bottom_y))))
			};
			vertices[1][2] = VertexFormat{
				data.pos + glm::vec4(data.vec1 + data.vec3, 0.0f),
				glm::vec2(float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Bottom_x))),
						  float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Bottom_y))))
			};
			vertices[1][3] = VertexFormat{
				data.pos + glm::vec4(data.vec1 + data.vec3, 0.0f),
				glm::vec2(float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Bottom_x))),
						  float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Bottom_y))))
			};
			vertices[1][4] = VertexFormat{
				data.pos + glm::vec4(data.vec3, 0.0f),
				glm::vec2(float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Bottom_x))),
						  float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Bottom_y))))
			};
			vertices[1][5] = VertexFormat{
			    data.pos,
			    glm::vec2(float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Bottom_x))),
					      float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Bottom_y))))
			};
			vertices[2][0] = VertexFormat{
	            data.pos + glm::vec4(data.vec2, 0.0f),
	            glm::vec2(float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Top_x))),
			              float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Top_y))))
			};
			vertices[2][1] = VertexFormat{
				data.pos + glm::vec4(data.vec2 + data.vec3, 0.0f),
				glm::vec2(float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Top_x))),
						  float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Top_y))))
			};
			vertices[2][2] = VertexFormat{
				data.pos + glm::vec4(data.vec2 + data.vec3 + data.vec1, 0.0f),
				glm::vec2(float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Top_x))),
						  float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Top_y))))
			};
			vertices[2][3] = VertexFormat{
				data.pos + glm::vec4(data.vec2 + data.vec3 + data.vec1, 0.0f),
				glm::vec2(float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Top_x))),
						  float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Top_y))))
			};
			vertices[2][4] = VertexFormat{
				data.pos + glm::vec4(data.vec2 + data.vec1, 0.0f),
				glm::vec2(float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Top_x))),
						  float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Top_y))))
			};
			vertices[2][5] = VertexFormat{
			    data.pos + glm::vec4(data.vec2, 0.0f),
			    glm::vec2(float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Top_x))),
					      float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Top_y))))
			};
			vertices[3][0] = VertexFormat{
				data.pos + glm::vec4(data.vec1, 0.0f),
				glm::vec2(float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Right_x))),
						  float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Right_y))))
			};
			vertices[3][1] = VertexFormat{
				data.pos + glm::vec4(data.vec1 + data.vec2, 0.0f),
				glm::vec2(float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Right_x))),
						  float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Right_y))))
			};
			vertices[3][2] = VertexFormat{
				data.pos + glm::vec4(data.vec1 + data.vec2 + data.vec3, 0.0f),
				glm::vec2(float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Right_x))),
						  float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Right_y))))
			};
			vertices[3][3] = VertexFormat{
				data.pos + glm::vec4(data.vec1 + data.vec2 + data.vec3, 0.0f),
				glm::vec2(float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Right_x))),
						  float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Right_y))))
			};
			vertices[3][4] = VertexFormat{
				data.pos + glm::vec4(data.vec1 + data.vec3, 0.0f),
				glm::vec2(float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Right_x))),
						  float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Right_y))))
			};
			vertices[3][5] = VertexFormat{
				data.pos + glm::vec4(data.vec1, 0.0f),
			    glm::vec2(float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Right_x))),
					      float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Right_y))))
			};
			vertices[4][0] = VertexFormat{
	        data.pos,
	            glm::vec2(float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Left_x))),
			              float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Left_y))))
			};
			vertices[4][1] = VertexFormat{
				data.pos + glm::vec4(data.vec3, 0.0f),
				glm::vec2(float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Left_x))),
						  float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Left_y))))
			};
			vertices[4][2] = VertexFormat{
				data.pos + glm::vec4(data.vec3 + data.vec2, 0.0f),
				glm::vec2(float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Left_x))),
						  float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Left_y))))
			};
			vertices[4][3] = VertexFormat{
				data.pos + glm::vec4(data.vec3 + data.vec2, 0.0f),
				glm::vec2(float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Left_x))),
						  float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Left_y))))
			};
			vertices[4][4] = VertexFormat{
				data.pos + glm::vec4(data.vec2, 0.0f),
				glm::vec2(float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Left_x))),
						  float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Left_y))))
			};
			vertices[4][5] = VertexFormat{
			    data.pos,
			    glm::vec2(float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Left_x))),
					      float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Left_y))))
			};
			vertices[5][0] = VertexFormat{
				data.pos + glm::vec4(data.vec3, 0.0f),
				glm::vec2(float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Front_x))),
						  float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Front_y))))
			};
			vertices[5][1] = VertexFormat{
				data.pos + glm::vec4(data.vec3 + data.vec1, 0.0f),
				glm::vec2(float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Front_x))),
						  float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Front_y))))
			};
			vertices[5][2] = VertexFormat{
				data.pos + glm::vec4(data.vec3 + data.vec1 + data.vec2, 0.0f),
				glm::vec2(float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Front_x))),
						  float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Front_y))))
			};
			vertices[5][3] = VertexFormat{
				data.pos + glm::vec4(data.vec3 + data.vec1 + data.vec2, 0.0f),
				glm::vec2(float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Front_x))),
						  float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Front_y))))
			};
			vertices[5][4] = VertexFormat{
				data.pos + glm::vec4(data.vec3 + data.vec2, 0.0f),
				glm::vec2(float(!bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Front_x))),
						  float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Front_y))))
			};
			vertices[5][5] = VertexFormat{
			    data.pos + glm::vec4(data.vec3, 0.0f),
			    glm::vec2(float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Front_x))),
					      float(bool(uint32_t(data.texture_flags) & uint32_t(TextureFlagBits::Front_y))))
			};
			glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(VertexFormat) * 6 * 6, &vertices[0]);
		}
	}
}

