#pragma once

#include "..\Model.h"
#include <glm\glm.hpp>

namespace Engine {
	namespace Rendering {
		namespace Models {
			class Triangle: public Model {
			public:
				Triangle(Core::State* parent);
				~Triangle() = default;
				struct VertexFormat {
					glm::vec3 position;
					glm::vec4 color;
				};
				VertexFormat VertexData[3]{};
				void Draw() override;
				void Update() override;
				GLuint position_array_attribute = -1;
				GLuint color_array_attribute = -1;

			};
		}
	}
}
