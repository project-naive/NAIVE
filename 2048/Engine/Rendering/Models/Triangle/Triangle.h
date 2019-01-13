#pragma once

#include "..\Model.h"
#include <glm\glm.hpp>

namespace Rendering {
	namespace Models {
		class Triangle: Model {
		public:
			Triangle();
			~Triangle() = default;
			struct VertexFormat {
				glm::vec3 position;
				glm::vec4 color;
			};
			VertexFormat VertexData[3]{};
			void Draw() override;
			GLuint position_array_attribute = -1;
			GLuint color_array_attribute = -1;
		};
	}
}

