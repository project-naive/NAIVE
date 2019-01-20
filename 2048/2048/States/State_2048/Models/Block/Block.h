#pragma once


#include "BlockShader.h"

#include <Graphics\Models\Model.h>

#include <glm\glm.hpp>

namespace State_2048 {
	namespace Models {
		class Block: public Engine::Graphics::Model {
		public:
			Block(Engine::Graphics::Managers::Shader& given_manager, size_t shaderID);
			struct DataFormat {
				glm::vec2 Block_center;
				glm::vec2 Block_size;
				glm::vec4 color;
				glm::mat4 projection;
				float     radius;
			};
			DataFormat data;
			GLuint Begin() override final;
			virtual void Draw() override final;
			virtual void Update() override final;
		private:
			typedef glm::vec4 VertexFormat;
			VertexFormat vertices[6] {
				glm::vec4(glm::vec2(), -1.0f,  1.0f),
				glm::vec4(glm::vec2(),  1.0f,  1.0f),
				glm::vec4(glm::vec2(),  1.0f, -1.0f),
				glm::vec4(glm::vec2(), -1.0f,  1.0f),
				glm::vec4(glm::vec2(), -1.0f, -1.0f),
				glm::vec4(glm::vec2(),  1.0f, -1.0f),
			};
			GLint projection_uniform_location;
			GLint color_uniform_location;
			GLint radius_uniform_location;
		};
	}
}
