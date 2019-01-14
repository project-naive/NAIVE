#pragma once

#include "..\Shader.h"

namespace Engine{
	namespace Graphics{
		namespace Shaders{
			class Text: public Shader{
				Text() {
					CreateProgram();
				}
				GLuint CreateProgram() override;
			};
		}
	}
}
