#pragma once

#include "FanShader.h"
#include "..\..\State_2048.h"

#include <Graphics\Models\Model.h>

#include <glm\glm.hpp>

namespace State_2048{
	namespace Models {
		class Fan: public Engine::Graphics::Model {
		public:
			Fan(Engine::Graphics::Managers::Shader& given_manager, size_t shaderID);
			struct DataFormat {
				glm::vec4 origin_color;
				glm::vec4 color1;
				glm::vec4 color2;
				glm::vec4 far_color;
				glm::vec4 origin;
				glm::vec3 vec1;
				glm::vec3 vec2;
			};
			DataFormat data;
			glm::mat4 projection;
			GLuint Begin() override final;
			virtual void Draw() override final;
			virtual void Update() override final;
		private:
			struct VertexFormat {
				glm::vec4 position;
				glm::vec4 color;
				glm::vec4 uv_coords;
			};
			VertexFormat VertexData[6];
//			Shaders::Fan* FanShader = nullptr;
			GLint projection_uniform_location;
		};
	}
}
