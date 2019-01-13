#include "ShaderManager.h"

#include "..\Shaders\Naive\Naive.h"

namespace Rendering {
	namespace Manager {
		::Rendering::Shader* Shader::shaders[NumOfTypes]{};
		Shader::Shader() {
			shaders[Naive] = new ::Rendering::Shaders::Naive;
		}
		Shader::~Shader() {
			for (size_t i = 0; i < NumOfTypes; i++) {
				delete shaders[i];
			}
		}
	}
}


