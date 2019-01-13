#pragma once

#include "..\Shaders\Shader.h"

namespace Rendering {
	namespace Manager {
		//A class that manages all the
		class Shader {
		public:
			enum ShaderType { Naive, NumOfTypes };
			static GLuint GetProgram(ShaderType type) {
				if (!shaders || type >= NumOfTypes || !shaders[type]) return 0;
				return shaders[type]->program;
			}
		private:
			static ::Rendering::Shader* shaders[NumOfTypes];
		public:
			Shader();
			~Shader();
		};
	}
}
