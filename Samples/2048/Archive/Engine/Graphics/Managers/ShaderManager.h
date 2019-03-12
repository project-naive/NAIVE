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
				enum ShaderType { Naive, Text, NumOfTypes, NonDefault, NoShader };
				GLuint UseDefault(ShaderType type){
					if(current != type){
						current = type;
						ActiveProgram = DefaultShaders[type]->program;
						glUseProgram(ActiveProgram);
					}
					return ActiveProgram;
				}
				ShaderType current = NoShader;
				GLuint UseGeneric(GLuint ID){
					if(ID < NumOfTypes){
						return UseDefault(ShaderType(ID));
					}
					if(ID == ActiveProgram) return Generics[ID - NumOfTypes];
					if(ID != ActiveProgram && ID < shader_count){
						glUseProgram(Generics[ID - NumOfTypes]);
						return Generics[ID - NumOfTypes];
					}
					return 0;
				}
			private:
				size_t shader_count = NumOfTypes;
				size_t shader_cache = 0;
				GLuint* Generics = nullptr;
				size_t unload_count = 0;
				size_t unload_cache = 0;
				size_t* unloaded = nullptr;
				GLuint ActiveProgram = 0;
				Shader(const Shader&)=delete;
				Shader& operator=(const Shader&)=delete;
				Graphics::Shader* DefaultShaders[NumOfTypes] {};
			};
		}
	}
}