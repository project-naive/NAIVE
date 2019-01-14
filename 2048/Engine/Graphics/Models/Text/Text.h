#pragma once

#include "..\..\Text\FTF.h"
#include "..\Model.h"
#include <glm\glm.hpp>

namespace Engine {
	namespace Graphics {
		namespace Models {
			class Text: public Model {
			public:
				Text(const Managers::Shader& given_manager);
				~Text() = default;
				void Draw() override;
				void Update() override;
			};
		}
	}
}
