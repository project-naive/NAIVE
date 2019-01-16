/*
*	Singleton class that manages OpenGL Shader Programs
*	For reusing same proram across different models
*	OpenGL is a state machine, thus a global coordination is required
*/

#pragma once

#include "..\..\Shaders\Shader.h"

namespace Engine {
	namespace Graphics {
		namespace Managers {
			//A class that manages all the shaders
			//TO-DO Add Generic shader management.
			class Shader {
			public:
				Shader();
				~Shader();
				enum ShaderType { Naive, Text, NumOfDefault, NoShader, NonDefault, NotManaged, ReserveField };
				GLuint UseDefault(ShaderType type){
					if(type>=NumOfDefault) return -1;
					if(current != type){
						current = type;
						ActiveProgram = DefaultShaders[type]->program;
						glUseProgram(ActiveProgram);
					}
					return ActiveProgram;
				}
				ShaderType current = NoShader;
				//Uses a program by calling its corresponding glUseProgram
				//Checks if current one is the given one
				GLuint UseGeneric(size_t ID){
					if (ID == size_t(-1)) return -1;
					if(ID < NumOfDefault){
						return UseDefault(ShaderType(ID));
					}
					if(ID < ReserveField) return GLuint(ID);
					if(Generics[ID - ReserveField]->program == ActiveProgram) 
						return ActiveProgram;
					if(ID - ReserveField < shader_count - NumOfDefault){
						ActiveProgram = Generics[ID - ReserveField]->program;
						glUseProgram(ActiveProgram);
						return ActiveProgram;
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
				Shader(const Shader&)=delete;
				Shader& operator=(const Shader&)=delete;
				Graphics::Shader* DefaultShaders[NumOfDefault] {};
			};
		}
	}
}