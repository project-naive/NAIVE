/*
*	Singleton class that manages OpenGL Shader Programs
*	For reusing same proram across different models
*	OpenGL is a state machine, thus a global coordination is required
*/

#pragma once

#include "..\Shaders\Shader.h"

namespace Engine {
	namespace Graphics {
		namespace Managers {
			//A class that manages all the
			class Shader {
			public:
				Shader();
				~Shader();
				enum ShaderType { Naive, NumOfTypes };
				GLuint GetProgram(ShaderType type) {
					if (!DefaultShaders || type >= NumOfTypes || !DefaultShaders[type]) return 0;
					return DefaultShaders[type]->program;
				}
			private:
				Shader(const Shader&)=delete;
				Shader& operator=(const Shader&)=delete;
				Engine::Graphics::Shader* DefaultShaders[NumOfTypes]{};
			};
		}
	}
}