#include "ShaderManager.h"

#include "..\Shaders\Naive\Naive.h"

namespace Engine {
	namespace Rendering {
		namespace Managers {
			void Shader::Init() {
				DefaultShaders[Naive] = new ::Engine::Rendering::Shaders::Naive;
			}
			void Shader::Destroy() {
				for (size_t i = 0; i < Shader::NumOfTypes; i++) {
					delete DefaultShaders[i];
					DefaultShaders[i] = nullptr;
				}
			}
		}
	}
}

