#pragma once

#include "..\Model.h"
#include <glm\glm.hpp>

namespace Engine {
	namespace Graphics {
		namespace Models {
			class Triangle: public Model {
			public:
				Triangle(Managers::Shader& given_manager);
				~Triangle() = default;
				struct VertexFormat {
					glm::vec3 position;
					glm::vec4 color;
				};
				VertexFormat VertexData[3] {};
				void Draw() override;
				void Update() override;
				GLuint Begin() override;
			};
		}
	}
}
