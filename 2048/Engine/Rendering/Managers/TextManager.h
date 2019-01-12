#pragma once

#include "..\Text\FTF.h"

#include <GL\glew.h>
#include <glm\glm.hpp>

namespace Rendering {
	namespace Manager {
		class Text {
		public:
			FTFonts fonts;
			struct Glyph_Texture {
				GLuint TextureID;
				glm::ivec2 size;
				glm::ivec2 bearing;
				GLuint advance;
			};
		};
	}
}


