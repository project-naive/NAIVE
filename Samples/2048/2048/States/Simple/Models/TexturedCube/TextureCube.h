#pragma once

#pragma once

#include <Graphics\Models\Model.h>
#include <glm\glm.hpp>
#include "..\TextureQuad\TQShader.h"

namespace State_2048 {
	namespace Models {
		class TextureCube: public Engine::Graphics::Model {
		public:
			TextureCube(Engine::Graphics::Managers::Shader& given_manager, size_t shaderID);
			GLuint Begin() override final;
			void Draw() override final;
			void Update() override final;
			enum class TextureFlagBits: 
				uint32_t{
				Default  = 0x0000,
				Back_x   = 0x0001,
				Back_y   = 0x0002,
				Bottom_x = 0x0004,
				Bottom_y = 0x0008,
				Top_x    = 0x0010,
				Top_y    = 0x0020,
				Right_x  = 0x0040,
				Right_y  = 0x0080,
				Left_x   = 0x0100,
				Left_y   = 0x0200,
				Front_x  = 0x0400,
				Front_y  = 0x0800
			};
			struct DataFormat {
				glm::vec4 pos;
				glm::vec3 vec1;
				glm::vec3 vec2;
				glm::vec3 vec3;
				GLuint textureID;
				TextureFlagBits texture_flags = TextureFlagBits::Default;
				glm::mat4 projection;
			};
			DataFormat data;
		private:
			struct VertexFormat {
				glm::vec4 position;
				glm::vec2 tex_coords;
			};
			GLint projection_uniform_location;
			VertexFormat vertices[6][6];
		};
	}
}

