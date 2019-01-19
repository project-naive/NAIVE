/*
*	Singleton class that manages OpenGL Shader Programs
*	For reusing same proram across different models
*	OpenGL is a state machine, thus a global coordination is required
*/

#pragma once

#include "..\..\Shaders\Shader.h"

namespace Engine {
	namespace Graphics {
		class Shader;
		namespace Managers {
			class Context;
			//A class that manages all the shaders
			//TO-DO Add Generic shader management.
			class Shader {
			public:
				Shader(const Managers::Context& ContextManager);
				~Shader();
				Shader(const Shader&) = delete;
				Shader& operator=(const Shader&) = delete;
				enum ShaderType { Naive, Text, NumOfDefault, NoShader, NonDefault, NotManaged, ReserveField };
				GLuint UseDefault(ShaderType type);
				ShaderType current = NoShader;
				//Uses a program by calling its corresponding glUseProgram
				//Checks if current one is the given one
				GLuint UseGeneric(size_t ID);
				GLuint GetCurrent() const {
					return ActiveProgram;
				}
				GLuint GetDefault(ShaderType type) const {
					if (type >= NumOfDefault) return -1;
					return DefaultShaders[type]->program;
				}
				GLuint GetGeneric(size_t ID) const {
					if (ID < NumOfDefault) {
						return DefaultShaders[ID]->program;
					}
					if (ID - ReserveField < shader_count - NumOfDefault && Generics[ID - ReserveField]) {
						return Generics[ID - ReserveField]->program;
					}
					return -1;
				}
				//Returns a shader index to be used for identification.
				//Adds the shader to tracking
				size_t addGeneric(Graphics::Shader& shader);
				bool delGeneric(size_t ID);
			private:
				size_t shader_count = NumOfDefault;
				size_t shader_cache = 0;
				Graphics::Shader** Generics = nullptr;
				size_t unload_count = 0;
				size_t unload_cache = 0;
				size_t* unloaded = nullptr;
				GLuint ActiveProgram = -1;
				size_t ActiveContext = 0;
				const Graphics::Shader* DefaultShaders[NumOfDefault] {};
				const Context& Contexts;
			};
		}
	}
}