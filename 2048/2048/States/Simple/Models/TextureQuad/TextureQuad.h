#pragma once

#include <Graphics\Models\Model.h>
#include <glm\glm.hpp>
#include "TQShader.h"

namespace State_2048 {
	namespace Models {
		class TextureQuad: public Engine::Graphics::Model {
		public:
			TextureQuad(Engine::Graphics::Managers::Shader& given_manager, size_t shaderID);
			GLuint Begin() override final;
			void Draw() override final;
			void Update() override final;
			struct DataFormat {
				glm::vec4 pos;
				glm::vec3 vec1;
				glm::vec3 vec2;
				GLuint textureID;
				glm::mat4 projection;
			};
			DataFormat data;
		private:
			struct VertexFormat{
				glm::vec4 position;
				const glm::vec2 tex_coords;
			};
			GLint projection_uniform_location;
			VertexFormat vertices[6]{ 
				{
					glm::vec4(),
					glm::vec2(0.0f, 0.0f)
				},{
					glm::vec4(),
					glm::vec2(1.0f, 0.0f)
				},{
					glm::vec4(),
					glm::vec2(1.0f, 1.0f)
				},{
					glm::vec4(),
					glm::vec2(1.0f, 1.0f)
				},{
					glm::vec4(),
					glm::vec2(0.0f, 1.0f)
				},{
					glm::vec4(),
					glm::vec2(0.0f, 0.0f)
				} };
		};
	}
}

