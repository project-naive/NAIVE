#include "ShaderManager.h"

#include "..\..\Shaders\Naive\Naive.h"
#include "..\..\Shaders\Text\TextShader.h"

#include "..\Globals\ContextManager.h"

#include <algorithm>

namespace Engine {
	namespace Graphics {
		namespace Managers {
			Shader::Shader(const Context& ContextManager):
				Contexts(ContextManager),
				DefaultShaders{ new Shaders::Naive(*this), new Shaders::Text(*this) } {
			}
			Shader::~Shader() {
				for (size_t i = 0; i < Shader::NumOfDefault; i++) {
					delete DefaultShaders[i];
					DefaultShaders[i] = nullptr;
				}
				for (size_t i = 0; i < shader_count - NumOfDefault; i++) {
					delete Generics[i];
				}
				delete[] Generics;
			}


			//Uses a program by calling its corresponding glUseProgram
			//Checks if current one is the given one
			GLuint Managers::Shader::UseDefault(ShaderType type) {
				if (type >= NumOfDefault) return -1;
				size_t cur_context = Contexts.GetCurrent();
				if (current == type && ActiveContext == cur_context) {
					return ActiveProgram;
				}
				ActiveContext = cur_context;
				current = type;
				ActiveProgram = DefaultShaders[type]->program;
				glUseProgram(ActiveProgram);
				return ActiveProgram;
			}
			GLuint Shader::UseGeneric(size_t ID) {
				if (ID == size_t(-1)) return -1;
				if (ID < NumOfDefault) {
					return UseDefault(ShaderType(ID));
				}
				if (ID < ReserveField) return -1;
				if (ID - ReserveField < shader_count - NumOfDefault && Generics[ID - ReserveField]) {
					size_t cur_context = Contexts.GetCurrent();
					if (Generics[ID - ReserveField]->program == ActiveProgram 
						&& cur_context == ActiveContext) {
						current = NonDefault;
						return ActiveProgram;
					}
					ActiveContext = cur_context;
					ActiveProgram = Generics[ID - ReserveField]->program;
					glUseProgram(ActiveProgram);
					current = NonDefault;
					return ActiveProgram;
				}
				return -1;
			}

			size_t Shader::addGeneric(Graphics::Shader& shader) {
				if (!shader.program) return NoShader;
				if (&shader.parent != this) return NotManaged;
				if (unload_count) {
					size_t rtn = unloaded[unload_count - 1];
					Generics[rtn - ReserveField] = &shader;
					unload_count--;
					return rtn;
				}
				if (shader_count - NumOfDefault == shader_cache) {
					size_t new_size = size_t(shader_cache * 1.618);
					new_size++;
					Graphics::Shader** new_cache = new Graphics::Shader*[new_size];
					for (size_t i = 0; i < shader_count - NumOfDefault; i++) {
						new_cache[i] = Generics[i];
					}
					std::swap(Generics, new_cache);
					shader_cache = new_size;
					delete[] new_cache;
				}
				Generics[shader_count - NumOfDefault] = &shader;
				shader_count++;
				return shader_count - NumOfDefault + ReserveField - 1;
			}

			bool Shader::delGeneric(size_t ID) {
				if(ID < ReserveField) return false;
				if (ID - ReserveField >= shader_count - NumOfDefault) {
					return false;
				}
				if (!Generics[ID - ReserveField]) return false;
				if (unload_count == unload_cache) {
					size_t new_size = size_t(unload_cache*1.618);
					new_size++;
					size_t* new_cache = new size_t[new_size];
					for (size_t i = 0; i < unload_count; i++) {
						new_cache[i] = unloaded[i];
					}
					std::swap(new_cache, unloaded);
					unload_cache = new_size;
					delete[] new_cache;
				}
				delete Generics[ID - ReserveField];
				Generics[ID - ReserveField] = nullptr;
				unloaded[unload_count] = ID;
				unload_count++;
				return true;
			}
		}
	}
}

