#pragma once

#include "..\..\Text\FTF.h"
#include "..\Model.h"
#include <glm\glm.hpp>

namespace Engine {
	namespace Graphics {
		namespace Models {
			class Text: public Model {
			public:
				Text(Managers::Shader& given_manager);
				~Text() = default;
				struct VertexFormat {
					GLfloat x, y;
					GLfloat tex_x, tex_y;
				};
				VertexFormat* VetexData = nullptr;
				void Draw() override;
				void Update() override;
				GLuint Begin() override;
			};
		}
	}
}
