#include "ShaderManager.h"

#include "..\Shaders\Naive\Naive.h"

namespace Engine {
	namespace Graphics {
		namespace Managers {
			Shader::Shader() {
				DefaultShaders[Naive] = new ::Engine::Graphics::Shaders::Naive;
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

