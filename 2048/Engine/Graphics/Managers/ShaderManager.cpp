#include "ShaderManager.h"

#include "..\Shaders\Naive\Naive.h"
#include "..\Shaders\Text\TextShader.h"

namespace Engine {
	namespace Graphics {
		namespace Managers {
			Shader::Shader() {
				DefaultShaders[Naive] = new Shaders::Naive;
				DefaultShaders[Text] = new Shaders::Text;
			}
			Shader::~Shader() {
				for (size_t i = 0; i < Shader::NumOfTypes; i++) {
					delete DefaultShaders[i];
					DefaultShaders[i] = nullptr;
				}
			}
		}
	}
}

