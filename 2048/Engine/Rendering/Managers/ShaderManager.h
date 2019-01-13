/*
*	Singleton class that manages OpenGL Shader Programs
*	For reusing same proram across different models
*	OpenGL is a state machine, thus a global coordination is required
*/

#pragma once

#include "..\Shaders\Shader.h"
#include "Manager.h"

namespace Engine {
	namespace Rendering {
		namespace Managers {
			//A class that manages all the
			class Shader: public Manager {
			public:
				enum ShaderType { Naive, NumOfTypes };
				GLuint GetProgram(ShaderType type) {
					if (!DefaultShaders || type >= NumOfTypes || !DefaultShaders[type]) return 0;
					return DefaultShaders[type]->program;
				}
			private:
				Engine::Rendering::Shader* DefaultShaders[NumOfTypes]{};
			public:
				void Init() override;
				void Destroy() override;
			};
		}
	}
}